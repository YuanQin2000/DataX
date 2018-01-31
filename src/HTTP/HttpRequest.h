/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <memory>
#include <sys/socket.h>
#include "Common/Typedefs.h"
#include "Common/ErrorNo.h"
#include "Common/Source.h"
#include "Common/Sink.h"
#include "DataCom/Configure.h"
#include "DataCom/ConnectionRunner.h"
#include "HTTPBase/Token.h"
#include "HTTPBase/HttpBaseRequest.h"
#include "URI/URI.h"
#include "HttpTokenDefs.h"
#include "HttpDefs.h"


using std::shared_ptr;

class CConnectionRunner;
class CPayloadDecoder;
class CConnectRequest;
class CHeaderField;
class CIOContext;

class CHttpRequest :
    public CHttpBaseRequest,
    public CSink
{
public:
    class IClient
    {
    public:
        enum HttpIndication {
            IND_HTTP_MIN = -1,
            IND_HTTP_URI_CREATED = 0,
            IND_HTTP_URI_UPDATED,
            IND_HTTP_URI_REDIRECT,
            IND_HTTP_URI_NOT_FOUND,
            IND_HTTP_URI_NOT_IMPLEMENTED,
            IND_HTTP_URI_TOO_LONG,
            IND_HTTP_ENTITY_TOO_LARGE,
            IND_HTTP_UNSUPOORT_MEDIA,
            IND_HTTP_REQUEST_PROCESSING,
            IND_HTTP_DOC_NO_UPDATED,
            IND_HTTP_MAX
        };

        struct RedirectData {
            const char* pLocation;
            bool bPermanently;

            RedirectData(const char* pLoc, bool bPerm) :
                pLocation(pLoc), bPermanently(bPerm) {}
        };

    public:
        virtual ~IClient() {}
        virtual void OnTerminated(ErrorCode status) = 0;
        virtual void OnData(uint8_t* pData, size_t len) = 0;

        virtual void OnHttpStatus(
            int code, const char* pStatusPhrase, const tStringMap* pExtField) = 0;
        virtual void OnHttpIndication(HttpIndication ind, void* pData = NULL) = 0;

        static const char* HttpIndicationName(HttpIndication ind);
    };

    // From CHttpBaseRequest(CRequest)
    ErrorCode OnPeerClosed();
    void OnTerminated(ErrorCode err);
    CIOContext* CreateIOContext(const sockaddr* pTarget);
    CConfigure& GetConfigure();

    // From CSink
    void OnSourceReady();

    ~CHttpRequest();

public:
    bool Start()
    {
        ASSERT(s_pHttpRunner);
        return s_pHttpRunner->PushRequest(this, this->GetPeerAddress());
    }

    CUri* GetTarget() const { return m_Target.get(); }
    const sockaddr* GetTargetAddress() const { return &m_TargetAddr; }
    const sockaddr* GetPeerAddress() const { return &m_PeerAddr; }
    bool ViaProxy() const { return m_bViaProxy; }
    bool IsSecure() const { return m_bSecure; }

    // Representation settings
    bool SetAccept();
    bool SetAcceptCharset();
    bool SetAcceptLang();

    static const char* GetErrorCodePhrase(ErrorCode code);
    static CHttpRequest* CreateInstance(
        IClient& client,
        CSource* pSource,
        RequestMethodID method,
        shared_ptr<CUri> pTarget);

private:
    CHttpRequest(
        IClient& client,
        CSource* pSource,
        RequestMethodID method,
        shared_ptr<CUri> pTarget);

    bool InitializeHeaderField();
    bool InitializeSocketAddress();
    void InitializeTarget();

    // From CHttpBaseRequest
    void OnReset();
    bool SerializePayload(uint8_t* pBuffer, size_t bufLen, size_t* pOutLen);
    ErrorCode HandleRespHeader(
        tTokenID versionID,
        int statusCode,
        const char* pStatusPhrase,
        CHeaderField* pHeaderField);
    ErrorCode ReceiveRespPayload(
        uint8_t* pData, size_t dataLen, size_t* pConsumedLen);

    bool AnalyzeRespHeaderField(
        tTokenID versionID,
        CHeaderField* pRespHeaderField,
        int* pOutContentLength,
        EncodingType* pOutType);
    bool AnalyzeEncodingType(CHeaderField* pRespHeaderField, EncodingType* pOutType);
    bool AnalyzeContentLength(tTokenID versionID, CHeaderField* pRespHeaderField, int* pOutContentLength);
    bool AnalyzeTransferEncoding(CHeaderField* pRespHeaderField, int* pOutContentLength);

    ErrorCode HandleStatusCode(int statusCode, CHeaderField* pRespHeaderField);
    ErrorCode Handle201Created(CHeaderField* pRespHF);
    ErrorCode Handle206PartialContent(CHeaderField* pRespHF);
    ErrorCode HandleRedirect(CHeaderField* pRespHF, bool bPermanently);
    ErrorCode Handle303SeeOther(CHeaderField* pRespHF);
    ErrorCode HandleUnauthorized(CHeaderField* pRespHF);

    CHttpRequest* CreateRedirectRequest(const char* pLocation);
    void HandleMultipleChoicePayload(uint8_t* pData, size_t length);

    static void HandlePayload(void* pThis, uint8_t* pData, size_t length);

    /**
     * @brief Handle the cookie from the response, update the cookie value in DB.
     */
    static void HandleCookie(CUri* pURI, CHeaderField* pRespHeaderField);

    static const char* GetStringFieldValue(CHeaderField* pHF, int fieldID);

private:
    class CHttpConfigure : public CConfigure
    {
    public:
        CHttpConfigure(size_t inBufSize, size_t outBufSize) :
            CConfigure(inBufSize, outBufSize) {}

        // From CConfigure
        bool CreateController(CController** pOutController, CRequest* pRequest);
    };

    class CHttpTunelConfigure : public CConfigure
    {
    public:
        CHttpTunelConfigure(size_t inBufSize, size_t outBufSize) :
            CConfigure(inBufSize, outBufSize) {}

        // From CConfigure
        bool CreateController(CController** pOutController, CRequest* pRequest);
    };

private:
    IClient& m_Client;
    const tTokenID m_MethodID;
    shared_ptr<CUri> m_Target;
    CSource* m_pSource;             // Not Owned
    CPayloadDecoder* m_pPayloadDecoder; // Owned
    CHttpRequest* m_pRedirectRequest;    // Owned
    int m_StatusCode;

    bool m_bViaProxy;
    bool m_bSecure;
    sockaddr m_TargetAddr;   // Orignial Server address.
    sockaddr m_PeerAddr;     // Direct connected address. (next hop)

    static CConnectionRunner* s_pHttpRunner;

    DISALLOW_COPY_CONSTRUCTOR(CHttpRequest);
    DISALLOW_ASSIGN_OPERATOR(CHttpRequest);
    DISALLOW_DEFAULT_CONSTRUCTOR(CHttpRequest);
};

#endif