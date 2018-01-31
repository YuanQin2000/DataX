/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpRequest.h"
#include "Common/ErrorNo.h"
#include "Common/Macros.h"
#include "IO/TCPClient.h"
#include "IO/SSLClient.h"
#include "Network/Address.h"
#include "Compress/CompressManager.h"
#include "HTTPBase/FieldValue.h"
#include "HTTPBase/PayloadParser.h"
#include "HttpUtils.h"
#include "HttpDefs.h"
#include "HttpTokenDefs.h"
#include "HttpToken.h"
#include "HttpHeaderFieldDefs.h"
#include "HttpFieldValue.h"
#include "HttpCookieManager.h"
#include "HttpConnectSession.h"
#include "HttpPrefManager.h"
#include "HttpProxyPref.h"
#include "HttpRequestPref.h"
#include "Tracker/Trace.h"
#include <errno.h>
#include <cstring>
#include <vector>
#include <new>

using std::memcpy;
using std::strlen;
using std::vector;

CConnectionRunner* CHttpRequest::s_pHttpRunner =
    CConnectionRunner::CreateInstance("default-http-client-stack");

CHttpRequest::CHttpRequest(
    IClient& client,
    CSource* pSource,
    RequestMethodID method,
    shared_ptr<CUri> pTarget) :
    CHttpBaseRequest(
        method,
        HTTP_VERSION_1_1,
        CHttpHeaderFieldDefs::GetResponseGlobalConfig(),
        true,
        NSHttpUtils::IsIdempotentMethod(method)),
    CSink(),
    m_Client(client),
    m_MethodID(method),
    m_Target(pTarget),
    m_pSource(pSource),
    m_pPayloadDecoder(NULL),
    m_pRedirectRequest(NULL),
    m_StatusCode(0),
    m_bViaProxy(false),
    m_bSecure(false)
{
    ASSERT(method >= 0 && method < REQUEST_METHOD_COUNT);
    ASSERT(pTarget.get());

    memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));
    memset(&m_TargetAddr, 0, sizeof(m_TargetAddr));
}

CHttpRequest::~CHttpRequest()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    if (m_pSource) {
        m_pSource->Close();
    }
    delete m_pPayloadDecoder;
    delete m_pRedirectRequest;
}

CHttpRequest* CHttpRequest::CreateInstance(
    IClient& client,
    CSource* pSource,
    RequestMethodID method,
    shared_ptr<CUri> pTarget)
{
    CHttpRequest* pInstance = new CHttpRequest(client, pSource, method, pTarget);
    if (pInstance) {
        if (pInstance->InitializeSocketAddress() &&
            pInstance->InitializeHeaderField()) {
            pInstance->InitializeTarget();
        } else {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

bool CHttpRequest::InitializeHeaderField()
{
    CHeaderField* pHeaderField =
        NSHttpUtils::CreateDefaultRequestHeaderField(GetBuffer());
    if (pHeaderField == NULL) {
        return false;
    }
    if (!NSHttpUtils::SetHostField(m_Target->Authority().get(), pHeaderField)) {
        delete pHeaderField;
        return false;
    }
    NSHttpUtils::SetCookieField(m_Target.get(), pHeaderField);
    SetRequestHeaderField(pHeaderField);
    return true;
}

bool CHttpRequest::InitializeSocketAddress()
{
    CAuthority* pAuthority = m_Target->Authority().get();
    if (pAuthority == NULL) {
        return false;
    }
    tNetworkAddress* pAddr = pAuthority->GetIPAddress();
    if (pAddr == NULL) {
        return false;
    }
    unsigned short targetPort = 0;
    switch (m_Target->Scheme()) {
    case SCHEME_HTTP:
        targetPort = DEFAULT_HTTP_PORT_NUM;
        m_bSecure = false;
        break;
    case SCHEME_HTTPS:
        targetPort = DEFAULT_HTTPS_PORT_NUM;
        m_bSecure = true;
        break;
    default:
        return false;
    }
    NSNetworkAddress::GetSockAddress(&m_TargetAddr, pAddr, pAuthority->GetPort(targetPort));
    CHttpProxyPref* pProxy = CHttpPrefManager::Instance()->GetConnectConfig().GetHttpProxy();
    if (pProxy && !pProxy->IsInWhiteList(pAddr)) {
        memcpy(&m_PeerAddr, pProxy->GetSocketAddress(), sizeof(m_PeerAddr));
        m_bViaProxy = true;
    } else {
        memcpy(&m_PeerAddr, &m_TargetAddr, sizeof(m_PeerAddr));
    }
    return true;
}

void CHttpRequest::InitializeTarget()
{
    if (m_bViaProxy && m_Target->Scheme() != SCHEME_HTTPS) {
        SetTarget(m_Target.get(), URI_ABSOLUTE_FORM, 0);
    } else {
        if (m_MethodID != REQUEST_METHOD_OPTIONS || m_Target->Path()) {
            SetTarget(m_Target.get(), URI_ORIGIN_FORM, 0);
        } else {
            SetTarget("*");     // OPTION on the server wide
        }
    }
}

void CHttpRequest::OnReset()
{
    if (m_pSource) {
        m_pSource->Reset();
    }
    CHttpBaseRequest::OnReset();
}

bool CHttpRequest::SerializePayload(
    uint8_t* pBuffer, size_t bufLen, size_t* pOutLen)
{
    *pOutLen = 0;
    if (m_pSource == NULL) {
        return true;
    }

    // TODO: 
    // 1) need to connect the source first.
    // 2) handle the source is not readable, 
    ASSERT(false);
    bool bFinished = false;
    if (m_pSource->IsReadable()) {
        int readLen = m_pSource->Read(pBuffer, bufLen);
        if (readLen > 0) {
            bFinished = m_pSource->IsEOF();
        } else if (readLen == 0) {
            bFinished = true;
        } else {
            OUTPUT_ERROR_TRACE("Read source failed\n");
            SET_ERROR_CODE(EC_IO_ERROR);
        }
    }
    return bFinished;
}

ErrorCode CHttpRequest::HandleRespHeader(
    tTokenID versionID,
    int statusCode,
    const char* pStatusPhrase,
    CHeaderField* pHeaderField)
{
#ifdef __DEBUG__
    pHeaderField->Dump();
#endif

    const tStringMap* pExtHeaderField = &(pHeaderField->GetExtensions().Map());
    if (pExtHeaderField->empty()) {
        pExtHeaderField = NULL;
    }
    ErrorCode error = HandleStatusCode(statusCode, pHeaderField);
    if (error != EC_SUCCESS) {
        return error;
    }
    m_Client.OnHttpStatus(statusCode, pStatusPhrase, pExtHeaderField);

    HandleCookie(m_Target.get(), pHeaderField);

    int contentLen = 0;
    EncodingType type = ENCODING_NONE;
    if (AnalyzeRespHeaderField(versionID, pHeaderField, &contentLen, &type)) {
        if (contentLen != 0) {
            if (m_pPayloadDecoder == NULL) {
                m_pPayloadDecoder = new CPayloadDecoder(
                    HandlePayload, this, contentLen, NSHttpUtils::Encoding2CompressType(type));
                if (m_pPayloadDecoder == NULL) {
                    error = EC_NO_MEMORY;
                }
            }
        }
    } else {
        error = EC_PROTOCOL_ERROR;
    }
    return error;
}

ErrorCode CHttpRequest::ReceiveRespPayload(
    uint8_t* pData, size_t dataLen, size_t* pConsumedLen)
{
    ASSERT(pData);
    ASSERT(pConsumedLen);

    if (m_pPayloadDecoder == NULL) {
        *pConsumedLen = 0;
        return EC_SUCCESS;  // No payload
    }

    ErrorCode resErr = EC_INPROGRESS;
    if (dataLen > 0) {
        resErr = m_pPayloadDecoder->Process(pData, dataLen, pConsumedLen);
    }
    return resErr;
}

void CHttpRequest::OnTerminated(ErrorCode err)
{
    if (m_pSource) {
        m_pSource->Close();
    }
    m_Client.OnTerminated(err);
}

ErrorCode CHttpRequest::OnPeerClosed()
{
    ErrorCode err = EC_SUCCESS;
    if (m_pPayloadDecoder) {
        if (!m_pPayloadDecoder->HandlePeerClosed()) {
            err = EC_PROTOCOL_ERROR;
        }
    }
    return err;
}

CIOContext* CHttpRequest::CreateIOContext(const sockaddr* pTarget)
{
    CTcpClient* pTcp = new CTcpClient(pTarget, false);
    CIOContext* pIO = pTcp;
    if (pTcp) {
        if (pTcp->Open()) {
            if (m_bSecure && !m_bViaProxy) {
                // Use SSL
                CIOContext* pTmp = pIO;
                pIO = CSSLClient::CreateInstance(pTmp, true);
                if (pIO == NULL) {
                    delete pTmp;
                }
            }
        } else {
            delete pIO;
            pIO = NULL;
        }
    }
    return pIO;
}

CConfigure& CHttpRequest::GetConfigure()
{
    static CHttpConnectionPref& connPreference(
        CHttpPrefManager::Instance()->GetConnectConfig());
    static CHttpConfigure s_HttpConfigure(
        connPreference.SendBufferSize(), connPreference.RecvBufferSize());
    static CHttpTunelConfigure s_HttpTunelConfigure(
        connPreference.SendBufferSize(), connPreference.RecvBufferSize());

    if (m_bSecure && m_bViaProxy) {
        return s_HttpTunelConfigure;
    } else {
        return s_HttpConfigure;
    }
}

// TODO: impl
void CHttpRequest::OnSourceReady()
{
    ASSERT(false);
}

bool CHttpRequest::SetAccept()
{
    return false;
}

bool CHttpRequest::SetAcceptCharset()
{
    return false;
}

bool CHttpRequest::SetAcceptLang()
{
    return false;
}

bool CHttpRequest::AnalyzeRespHeaderField(
    tTokenID versionID,
    CHeaderField* pRespHeaderField,
    int* pOutContentLength,
    EncodingType* pOutType)
{
    if (CHttpPrefManager::Instance()->GetRequestPref().Accept(*pRespHeaderField)) {
        if (!AnalyzeEncodingType(pRespHeaderField, pOutType)) {
            return false;
        }
        if (!AnalyzeContentLength(versionID, pRespHeaderField, pOutContentLength)) {
            return false;
        }
        if (!AnalyzeTransferEncoding(pRespHeaderField, pOutContentLength)) {
            return false;
        }
        return true;
    }
    return false;
}

bool CHttpRequest::AnalyzeEncodingType(CHeaderField* pRespHeaderField, EncodingType* pOutType)
{
    ASSERT(pOutType);

    bool bRes = true;
    CForwardList* pList =
        pRespHeaderField->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_CONTENT_ENCODING);
    if (pList) {
        ASSERT(pList->Count() > 0);
        if (pList->Count() > 1) {
            return false;
        }
        CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>* pEncodingObject =
                reinterpret_cast<CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>*>(pList->First());
        ASSERT(pEncodingObject);
        *pOutType = static_cast<EncodingType>(pEncodingObject->GetToken());
    }
    return bRes;
}

bool CHttpRequest::AnalyzeContentLength(
    tTokenID versionID, CHeaderField* pRespHeaderField, int* pOutContentLength)
{
    ASSERT(pOutContentLength);

    *pOutContentLength = 0;
    bool bRes = true;
    CForwardList* pList =
        pRespHeaderField->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_CONTENT_LENGTH);
    if (pList) {
        if (pList->Count() != 1) {
            return false;
        }
        CIntFieldValue* pIntObject = reinterpret_cast<CIntFieldValue*>(pList->First());
        ASSERT(pIntObject);
        *pOutContentLength = pIntObject->Value();
        if (*pOutContentLength < 0) {
            bRes = false;
        }
    } else {
        CForwardList* pConnectionOpts =
            pRespHeaderField->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_CONNECTION);
        if (pConnectionOpts == NULL) {
            if (versionID == HTTP_VERSION_1_0) {
                *pOutContentLength = CONTENT_LENGTH_UNKNOWN;
            }
        } else {
            CForwardList::Iterator iter = pConnectionOpts->Begin();
            CForwardList::Iterator iterEnd = pConnectionOpts->End();
            while (iter != iterEnd) {
                CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_CONNECTION>*
                    pValueObj = reinterpret_cast<CTokenIndexFieldValue<
                        CHttpTokenMap::CATEGORY_CONNECTION>*>(pConnectionOpts->DataAt(iter));
                tTokenID connectionOpts = pValueObj->GetToken();
                if (connectionOpts == CO_CLOSE) {
                    *pOutContentLength = CONTENT_LENGTH_UNKNOWN;
                    break;
                }
                ++iter;
            }
        }
    }
    return bRes;
}

bool CHttpRequest::AnalyzeTransferEncoding(
    CHeaderField* pRespHeaderField, int* pOutContentLength)
{
    ASSERT(pOutContentLength);

    bool bRes = true;
    CForwardList* pList =
        pRespHeaderField->GetFieldValueByID(
            CHttpHeaderFieldDefs::RESP_FN_TRANSFER_ENCODING);
    if (pList) {
        CForwardList::Iterator iter = pList->Begin();
        CForwardList::Iterator iterEnd = pList->End();
        while (iter != iterEnd) {
            CHttpTransferEncodingFieldValue* pTEObject =
                reinterpret_cast<CHttpTransferEncodingFieldValue*>(pList->DataAt(iter));
            ASSERT(pTEObject);
            if (pTEObject->GetValueType() == CHttpTransferEncodingFieldValue::VT_CHUNKED) {
                *pOutContentLength = CONTENT_LENGTH_CHUNKED;
            }
            ++iter;
        }
    }
    return bRes;
}

ErrorCode CHttpRequest::HandleStatusCode(int statusCode, CHeaderField* pRespHeaderField)
{
    if (statusCode < SC_MIN || statusCode > SC_MAX) {
        OUTPUT_WARNING_TRACE("Unknown status code: %d\n", statusCode);
        return EC_PROTOCOL_ERROR;
    }
    m_StatusCode = statusCode;

    ErrorCode error = EC_SUCCESS;
    switch (statusCode) {
    case SC100_CONTINUE:
    case SC101_SWITCH_PROTOCOL:
        ASSERT(false);
        break;
    case SC200_OK:
        break;
    case SC201_CREATED:
        error = Handle201Created(pRespHeaderField);
        break;
    case SC202_ACCEPTED:
        m_Client.OnHttpIndication(IClient::IND_HTTP_REQUEST_PROCESSING);
        break;
    case SC203_NON_AUTHORITATIVE:
        // The information contained in the entity headers came not from the
        // origin server but from a copy of the resource. This could happen
        // if an intermediary had a copy of a resource but could not or did not
        // validate the meta-information (headers) it sent about the resource.
        //
        // This response code is not required to be used; it is an option for
        // applications that have a response that would be a 200 status if the
        // entity headers had come from the origin server.
        break;
    case SC204_NO_CONTENT:
    case SC205_RESET_CONTENT:
        // Notify the status only, no content.
        // 205 told the browser need to refresh the input form.
        break;
    case SC206_PARTIAL_CONTENT:
        // For Range Request.
        error = Handle206PartialContent(pRespHeaderField);
        break;
    case SC300_MULTIPLE_CHOICE:
        // The choice list is in the payload.
        break;
    case SC301_MOVE_PERMANENTLY:
        error = HandleRedirect(pRespHeaderField, true);
        break;
    case SC302_FOUND:
    case SC307_TEMPORARY_REDIRECT:
        error = HandleRedirect(pRespHeaderField, false);
        break;
    case SC303_SEE_OTHER:
        error = Handle303SeeOther(pRespHeaderField);
        break;
    case SC304_NOT_MODIFIED:
        m_Client.OnHttpIndication(IClient::IND_HTTP_DOC_NO_UPDATED);
        break;
    case SC305_USE_PROXY:
        // Deprecated, so ignore this.
        break;
    case SC400_BAD_REQUEST:
        break;
    case SC401_UNAUTHORIZED:
    case SC407_PROXY_UNAUTHORIZED:
        error = HandleUnauthorized(pRespHeaderField);
        break;
    case SC402_PAYMENT_REQUIRED:
        // Reservered for futhure used.
        break;
    case SC403_FORBIDDEN:
        break;
    case SC404_NOT_FOUND:
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_NOT_FOUND);
        break;
    case SC405_METHOD_NOT_ALLOWED:
        break;
    case SC406_NOT_ACCEPTABLE:
        break;
    case SC408_REQUEST_TIMEOUT:
        break;
    case SC409_CONFLICT:
        break;
    case SC410_GONE:
        break;
    case SC411_LENGTH_REQUIRED:
        ASSERT(false);  // Client BUG found.
        break;
    case SC412_PRECONDITION_FAILED:
        break;
    case SC413_PAYLOAD_TOO_LARGE:
        m_Client.OnHttpIndication(IClient::IND_HTTP_ENTITY_TOO_LARGE);
        break;
    case SC414_URI_TOO_LONG:
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_TOO_LONG);
        break;
    case SC415_UNSUPPORTED_MEDIA_TYPE:
        m_Client.OnHttpIndication(IClient::IND_HTTP_UNSUPOORT_MEDIA);
        break;
    case SC416_RANGE_NOT_SATISFIABLE:
        ASSERT(false);  // Client BUG found.
        break;
    case SC417_EXPECTATION_FAILED:
        break;
    case SC426_UPGRAD_REQUIRED:
        break;
    case SC500_INTERNAL_SERVER_ERROR:
        break;
    case SC501_NOT_IMPLEMENTED:
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_NOT_IMPLEMENTED);
        break;
    case SC502_BAD_GATEWAY:
    case SC504_GATEWAY_TIMEOUT:
        error = EC_CONNECT_FAILED;
        break;
    case SC503_SERVICE_UNAVAILABLE:
        break;
    case SC505_HTTP_VERSION_NOT_SUPPORTED:
        ASSERT(false);  // Client BUG found.
        break;
    default:
        break;
    }
    return error;
}

ErrorCode CHttpRequest::Handle201Created(CHeaderField* pRespHF)
{
    char* pLocation = const_cast<char*>(GetStringFieldValue(
        pRespHF, CHttpHeaderFieldDefs::RESP_FN_LOCATION));
    if (pLocation) {
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_CREATED, pLocation);
    }
    return EC_SUCCESS;
}

ErrorCode CHttpRequest::Handle206PartialContent(CHeaderField* pRespHF)
{
    // TODO: Impl
    return EC_SUCCESS;
}

ErrorCode CHttpRequest::HandleRedirect(CHeaderField* pRespHF, bool bPermanently)
{
    ASSERT(m_pRedirectRequest == NULL);

    const char* pLocation = GetStringFieldValue(
        pRespHF, CHttpHeaderFieldDefs::RESP_FN_LOCATION);
    if (pLocation) {
        IClient::RedirectData redirectData(pLocation, bPermanently);
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_REDIRECT, &redirectData);
        m_pRedirectRequest = CreateRedirectRequest(pLocation);
        if (m_pRedirectRequest) {
            m_pRedirectRequest->Start();
        }
    }
    return EC_SUCCESS;
}

ErrorCode CHttpRequest::Handle303SeeOther(CHeaderField* pRespHF)
{
    ASSERT(m_pRedirectRequest == NULL);

    char* pLocation = const_cast<char*>(GetStringFieldValue(
        pRespHF, CHttpHeaderFieldDefs::RESP_FN_LOCATION));
    if (pLocation) {
        m_Client.OnHttpIndication(IClient::IND_HTTP_URI_UPDATED, pLocation);
        m_pRedirectRequest = CreateRedirectRequest(pLocation);
        if (m_pRedirectRequest) {
            m_pRedirectRequest->Start();
        }
    }
    return EC_SUCCESS;
}

ErrorCode CHttpRequest::HandleUnauthorized(CHeaderField* pRespHF)
{
    // TODO: impl
    return EC_SUCCESS;
}

void CHttpRequest::HandleMultipleChoicePayload(uint8_t* pData, size_t length)
{
    // TODO: Analyze the payload.
}

void CHttpRequest::HandlePayload(void* pThis, uint8_t* pData, size_t length)
{
    CHttpRequest* pRequest = reinterpret_cast<CHttpRequest*>(pThis);
    if (NSHttpUtils::IsResponseOK(pRequest->m_StatusCode)) {
        pRequest->m_Client.OnData(pData, length);
        return;
    }

    switch (pRequest->m_StatusCode) {
    case SC300_MULTIPLE_CHOICE:
        pRequest->HandleMultipleChoicePayload(pData, length);
        break;
    default:
        OUTPUT_NOTICE_TRACE("Ignore payload for status %d\n", pRequest->m_StatusCode);
        break;
    }
}

void CHttpRequest::HandleCookie(CUri* pURI, CHeaderField* pRespHeaderField)
{
    CForwardList* pCookies[2];
    pCookies[0] = pRespHeaderField->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_SET_COOKIE);
    pCookies[1] = pRespHeaderField->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_SET_COOKIE2);
    for (size_t i = 0; i < COUNT_OF_ARRAY(pCookies); ++i) {
        if (pCookies[i] == NULL) {
            continue;
        }
        CForwardList::Iterator iter = pCookies[i]->Begin();
        CForwardList::Iterator iterEnd = pCookies[i]->End();
        CHttpCookieManager* pMgr = CHttpCookieManager::GetDefaultInstance();
        while (iter != iterEnd) {
            CHttpSetCookieFieldValue* pSetCookieFV =
                reinterpret_cast<CHttpSetCookieFieldValue*>(pCookies[i]->DataAt(iter));
            if (!pMgr->SetCookie(pURI, pSetCookieFV->GetData())) {
                OUTPUT_WARNING_TRACE(
                    "Store Cookie Data failed for %s\n",
                    pURI->Authority()->HostName().pName);
            }
            ++iter;
        }
    }
}

const char* CHttpRequest::GetStringFieldValue(CHeaderField* pHF, int fieldID)
{
    CForwardList* pList =
        pHF->GetFieldValueByID(CHttpHeaderFieldDefs::RESP_FN_LOCATION);
    if (pList == NULL) {
        OUTPUT_WARNING_TRACE("Header Field have no header field: %d\n", fieldID);
        return NULL;
    }
    if (pList->Count() != 1) {
        OUTPUT_WARNING_TRACE("Header Field have wrong field value count: %d\n", pList->Count());
        return NULL;
    }

    CStringFieldValue* pStringObj = reinterpret_cast<CStringFieldValue*>(pList->First());
    ASSERT(pStringObj);
    return pStringObj->Value();
}

CHttpRequest* CHttpRequest::CreateRedirectRequest(const char* pLocation)
{
    CUriBuilder builder;
    CUri* pTarget = builder.CreateUriByString(NULL, pLocation);
    if (pTarget == NULL) {
        OUTPUT_WARNING_TRACE(
            "Create URI (%s) failed: %s\n", pLocation, builder.ErrorPhrase());
        return NULL;
    }
    shared_ptr<CUri> target(pTarget);
    return CreateInstance(
        m_Client, m_pSource, static_cast<RequestMethodID>(m_MethodID), target);
}


bool CHttpRequest::CHttpConfigure::CreateController(
    CController** pOutController, CRequest* pRequest)
{
    *pOutController = NULL;
    return true;
}

bool CHttpRequest::CHttpTunelConfigure::CreateController(
    CController** pOutController, CRequest* pRequest)
{
    CHttpRequest* pHttpReq = static_cast<CHttpRequest*>(pRequest);
    unsigned short port = pHttpReq->IsSecure() ? DEFAULT_HTTPS_PORT_NUM : 0;
    CHttpConnectSession* pController = CHttpConnectSession::CreateInstance(
        pHttpReq->GetTarget(), port, pHttpReq->IsSecure());
    if (pController) {
        *pOutController = pController;
        return true;
    }
    return false;
}


const char* CHttpRequest::IClient::HttpIndicationName(IClient::HttpIndication ind)
{
    ASSERT(ind > IClient::IND_HTTP_MIN);
    ASSERT(ind < IClient::IND_HTTP_MAX);

    static const char* s_IndicationName[] = {
        "URI Created",
        "URI Updated",
        "URI Redirect",
        "URI Not Found",
        "URI Not Implemented",
        "URI Too Long"
        "Entity Too Large",
        "Unsupported Media",
        "Request Processing",
        "Document Not Updated"
    };

    return s_IndicationName[ind];
}
