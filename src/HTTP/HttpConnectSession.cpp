/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpConnectSession.h"
#include "URI/URI.h"
#include "Common/CharHelper.h"
#include "HTTPBase/HeaderField.h"
#include "HTTPBase/HeaderParser.h"
#include "HttpTokenDefs.h"
#include "HttpDefs.h"
#include "HttpHeaderFieldDefs.h"
#include "HttpUtils.h"
#include "HttpRequestPref.h"
#include "HttpPrefManager.h"
#include "DataCom/Connection.h"

CHttpConnectSession::CHttpConnectSession(
    CUri* pUri, unsigned short port, bool bSecure) :
    m_Buffer(),
    m_State(STATE_INITIATE),
    m_bSecure(bSecure),
    m_RequestLine(
        REQUEST_METHOD_CONNECT, pUri, URI_AUTHORITY_FORM, port, HTTP_VERSION_1_1),
    m_pReqHeadField(NULL),
    m_pCurPos(m_RequestContent),
    m_pRequestContentEnd(NULL),
    m_RequestContent{0}
{
}

CHttpConnectSession::~CHttpConnectSession()
{
    delete m_pReqHeadField;
}

CHttpConnectSession* CHttpConnectSession::CreateInstance(
    CUri* pUri, unsigned short port, bool bSecure /* = true */)
{
    CHttpConnectSession* pInstance = new CHttpConnectSession(pUri, port, bSecure);
    if (pInstance == NULL) {
        return NULL;
    }

    char* pCurBuf = pInstance->m_RequestContent;
    size_t bufSize = sizeof(pInstance->m_RequestContent) - 1;
    size_t fillLen = 0;
    bool bComplete = false;
    CHeaderField::tSerializeAnchor anchor = 0;

    CHttpRequestPref& requestPref(CHttpPrefManager::Instance()->GetRequestPref());
    CHeaderField::FieldInitedValue* pInitValue = requestPref.GetFieldInitedValue();
    CForwardList* pUserAgentList = NULL;

    CHeaderField* pHeaderField = CHeaderField::CreateInstance(
        CHttpHeaderFieldDefs::GetConnectRequestGlobalConfig(), pInstance->m_Buffer);
    if (pHeaderField == NULL) {
        goto FAILED_EXIT;
    }

    pInstance->m_pReqHeadField = pHeaderField;
    if (pInitValue) {
        pUserAgentList = pInitValue->GetValue(CHttpHeaderFieldDefs::REQ_FN_USER_AGENT);
    }
    if (pUserAgentList) {
        bComplete = pHeaderField->SetFieldValue(
            CHttpHeaderFieldDefs::REQ_FN_USER_AGENT, pUserAgentList);
    } else {
        bComplete = pHeaderField->SetFieldValue(
            CHttpHeaderFieldDefs::REQ_FN_USER_AGENT, "httpclient");
    }
    if (!bComplete ||
        !NSHttpUtils::SetHostField(pUri->Authority().get(), pHeaderField)) {
        goto FAILED_EXIT;
    }

    // Step1: Serialize request line.
    bComplete = pInstance->m_RequestLine.Serialize(pCurBuf, bufSize, &fillLen);
    bufSize -= fillLen;
    pCurBuf += fillLen;
    if (!bComplete || bufSize == 0) {
        goto FAILED_EXIT;
    }

    // Step2: Serialize header field.
    anchor = pHeaderField->AnchorBegin();
    anchor = pHeaderField->Serialize(pCurBuf, bufSize, ": ", "\r\n", anchor, &fillLen);
    if (!pHeaderField->IsEnd(anchor)) {
        goto FAILED_EXIT;
    }
    pCurBuf += fillLen;
    bufSize -= fillLen;
    if (bufSize < 2) {
        goto FAILED_EXIT;
    }
    pCurBuf[0] = '\r';
    pCurBuf[1] = '\n';
    pCurBuf += 2;
    pInstance->m_pRequestContentEnd = pCurBuf;
    return pInstance;

FAILED_EXIT:
    delete pInstance;
    return NULL;
}

ErrorCode CHttpConnectSession::HandleData(
    uint8_t* pData, size_t dataLen, size_t* pConsumedLen)
{
    ErrorCode resErr = EC_UNKNOWN;
    switch (m_State) {
    case STATE_SUCCESS:
        *pConsumedLen = 0;
        resErr = EC_INACTIVE;
        break;
    case STATE_RECVING_RESP:
        resErr = HandleConnectResponse(pData, dataLen, pConsumedLen);
        if (resErr == EC_SUCCESS) {
            m_State = STATE_SUCCESS;
            if (m_bSecure) {
                if (!m_pConnect->ActivateSecure()) {
                    m_State = STATE_FAILED;
                    resErr = EC_CONNECT_FAILED;
                }
            }
        } else if (resErr != EC_INPROGRESS) {
            m_State = STATE_FAILED;
        }
        break;
    case STATE_FAILED:
    case STATE_INITIATE:
    case STATE_SENDING_INVITE:
        *pConsumedLen = 0;
        resErr = EC_PROTOCOL_ERROR;
        break;
    default:
        ASSERT(false);
        break;
    }
    return resErr;
}

ErrorCode CHttpConnectSession::GenerateData(
    uint8_t* pBuf, size_t bufLen, size_t* pOutLen)
{
    ErrorCode resErr = EC_UNKNOWN;
    switch (m_State) {
    case STATE_SUCCESS:
        *pOutLen = 0;
        resErr = EC_INACTIVE;
        break;
    case STATE_INITIATE:
    case STATE_SENDING_INVITE:
    {
        size_t remainLen = m_pRequestContentEnd - m_pCurPos;
        size_t copyLen = 0;
        if (remainLen > bufLen) {
            copyLen = bufLen;
            resErr = EC_INPROGRESS;
        } else {
            copyLen = remainLen;
            resErr = EC_SUCCESS;
            m_State = STATE_RECVING_RESP;
        }
        if (copyLen > 0) {
            memcpy(pBuf, m_pCurPos, copyLen);
            m_pCurPos += copyLen;
        }
        *pOutLen = copyLen;
        break;
    }
    case STATE_RECVING_RESP:
        *pOutLen = 0;
        resErr = EC_INPROGRESS;
        break;
    case STATE_FAILED:
        *pOutLen = 0;
        resErr = EC_PROTOCOL_ERROR;
        break;
    default:
        ASSERT(false);
        break;
    }
    return resErr;
}

bool CHttpConnectSession::OnPeerClosed()
{
    return m_State == STATE_SUCCESS;
}

void CHttpConnectSession::Reset()
{
    m_State = STATE_INITIATE;
    m_pCurPos = m_RequestContent;
}

ErrorCode CHttpConnectSession::HandleConnectResponse(
    uint8_t* pData, size_t dataLen, size_t* pConsumedLen)
{
    char* pCur = reinterpret_cast<char*>(pData);
    char* pEnd = pCur + dataLen;
    size_t curConsumed = 0;
    size_t totalConsumed = 0;
    ErrorCode resErr = EC_INPROGRESS;

    if (m_pStatusLine == NULL) {
        CHeaderParser parser(pCur, dataLen, &m_Buffer);
        resErr = parser.CreateStatusLine(&m_pStatusLine);
        curConsumed = parser.GetConsumedSize();
        if (m_pStatusLine == NULL) {
            *pConsumedLen = curConsumed;
            return resErr;
        }
        pCur += curConsumed;
        dataLen -= curConsumed;
        totalConsumed = curConsumed;
        curConsumed = 0;
    }

    // Parse the header field(just skip)
    const char* pCRLF =
        NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd);
    while (pCRLF) {
        curConsumed = pCRLF - pCur + 2;
        totalConsumed += curConsumed;
        if (pCRLF == pCur) {
            // Reach the end, then check the status line code.
            resErr =
                m_pStatusLine->GetStatusCode() == SC200_OK ?
                EC_SUCCESS : EC_CONNECT_FAILED;
            break;
        }
        pCur += curConsumed;
        pCRLF = NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd);
    }

    *pConsumedLen = totalConsumed;
    return resErr;
}
