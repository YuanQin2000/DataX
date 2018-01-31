/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_REQUEST_H__
#define __HTTP_BASE_REQUEST_H__

#include "Common/Typedefs.h"
#include "Common/ErrorNo.h"
#include "DataCom/Request.h"
#include "RequestLine.h"
#include "HeaderField.h"
#include "Token.h"
#include "URI/URI.h"

class CStatusLine;
class CHttpBaseRequest : public CRequest
{
public:
    enum SendRecvState {
        SR_INITAITED,
        SR_SENDING,
        SR_RECVING,
        SR_COMPLETE,
    };

    virtual ~CHttpBaseRequest();

    // From CRequest
    bool Serialize(uint8_t* pBuf, size_t bufLen, size_t* pOutLen);
    ErrorCode OnResponse(uint8_t* pData, size_t dataLen, size_t* pConsumedLen);
    virtual void OnReset();

    // Leave implemenation to the derived class.
    // virtual ErrorCode OnPeerClosed() = 0;
    // virtual void OnTerminated(ErrorCode err) = 0;
    // virtual CIOContext* CreateIOContext(const sockaddr* pTarget) = 0;
    // virtual CConfigure& GetConfigure() = 0;


    bool AppendExtHeaderField(const char* pName, const char* pValue)
    {
        return m_pReqHeaderField->SetExtFieldValue(pName, pValue);
    }

    CLazyBuffer& GetBuffer() { return m_Buffer; }

    void SetTarget(
        CUri* pTarget, tURISerializeOptions opts, unsigned short port = 0)
    {
        return m_RequestLine.SetTarget(pTarget, opts, port);
    }

    void SetTarget(const char* pString)
    {
        return m_RequestLine.SetTarget(pString);
    }

    void SetRequestHeaderField(CHeaderField* pHeaderField)
    {
        ASSERT(pHeaderField);
        ASSERT(m_pReqHeaderField == NULL);
        m_pReqHeaderField = pHeaderField;
        m_ReqHFAnchor = pHeaderField->AnchorBegin();
    }

protected:
    CHttpBaseRequest(
        tTokenID method,
        tTokenID version,
        const CHeaderField::GlobalConfig* pRespHFConfig,
        bool bHasResp,
        bool bPipeline);

    CHttpBaseRequest(
        tTokenID method,
        const char* pTarget,
        tTokenID version,
        const CHeaderField::GlobalConfig* pRespHFConfig,
        bool bHasResp,
        bool bPipeline);

    CHttpBaseRequest(
        tTokenID method,
        CUri* pTarget,
        tURISerializeOptions opts,
        unsigned short port,
        tTokenID version,
        const CHeaderField::GlobalConfig* pRespHFConfig,
        bool bHasResp,
        bool bPipeline);

private:
    virtual bool SerializePayload(
        uint8_t* pBuffer, size_t bufLen, size_t* pOutLen) = 0;
    virtual ErrorCode HandleRespHeader(
        tTokenID versionID,
        int statusCode,
        const char* pStatusPhrase,
        CHeaderField* pHeaderField) = 0;
    virtual ErrorCode ReceiveRespPayload(
        uint8_t* pData, size_t dataLen, size_t* pConsumedLen) = 0;

private:
    enum SerializeStatus {
        SERIALIZE_START_LINE,
        SERIALIZE_HEADER_FIELD,
        SERIALIZE_PAYLOAD,
        SERIALIZE_COMPLETE,
    };

    enum ProcessRespStatus {
        PROCESS_RESP_STATUS_LINE,
        PROCESS_RESP_HEADER_FIELD,
        PROCESS_RESP_PAYLOAD,
    };

    CRequestLine m_RequestLine;
    CHeaderField* m_pReqHeaderField;  // Owned
    CHeaderField::tSerializeAnchor m_ReqHFAnchor;

    const CHeaderField::GlobalConfig* m_pRespHFConfig;  // Not owned
    CStatusLine* m_pStatusLine;         // Not Owned directly (owned by m_Buffer)
    CHeaderField* m_pRespHeaderField;   // Owned

    SendRecvState m_State;
    SerializeStatus m_SerializeStatus;
    ProcessRespStatus m_ProcessRespStatus;

    CLazyBuffer m_Buffer;

    DISALLOW_COPY_CONSTRUCTOR(CHttpBaseRequest);
    DISALLOW_ASSIGN_OPERATOR(CHttpBaseRequest);
    DISALLOW_DEFAULT_CONSTRUCTOR(CHttpBaseRequest);
};

#endif