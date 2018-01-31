/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpBaseRequest.h"
#include "TokenDefs.h"
#include "HeaderParser.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"
#include <cstdio>

CHttpBaseRequest::CHttpBaseRequest(
    tTokenID method,
    tTokenID version,
    const CHeaderField::GlobalConfig* pRespHFConfig,
    bool bHasResp,
    bool bPipeline) :
    CRequest(bHasResp, bPipeline),
    m_RequestLine(method, version),
    m_pReqHeaderField(NULL),
    m_ReqHFAnchor(0),
    m_pRespHFConfig(pRespHFConfig),
    m_pStatusLine(NULL),
    m_pRespHeaderField(NULL),
    m_State(SR_INITAITED),
    m_SerializeStatus(SERIALIZE_START_LINE),
    m_ProcessRespStatus(PROCESS_RESP_STATUS_LINE)
{
}

CHttpBaseRequest::CHttpBaseRequest(
    tTokenID method,
    const char* pTarget,
    tTokenID version,
    const CHeaderField::GlobalConfig* pRespHFConfig,
    bool bHasResp,
    bool bPipeline) :
    CRequest(bHasResp, bPipeline),
    m_RequestLine(method, pTarget, version),
    m_pReqHeaderField(NULL),
    m_ReqHFAnchor(0),
    m_pRespHFConfig(pRespHFConfig),
    m_pStatusLine(NULL),
    m_pRespHeaderField(NULL),
    m_State(SR_INITAITED),
    m_SerializeStatus(SERIALIZE_START_LINE),
    m_ProcessRespStatus(PROCESS_RESP_STATUS_LINE)
{
}

CHttpBaseRequest::CHttpBaseRequest(
    tTokenID method,
    CUri* pTarget,
    tURISerializeOptions opts,
    unsigned short port,
    tTokenID version,
    const CHeaderField::GlobalConfig* pRespHFConfig,
    bool bHasResp,
    bool bPipeline) :
    CRequest(bHasResp, bPipeline),
    m_RequestLine(method, pTarget, opts, port, version),
    m_pReqHeaderField(NULL),
    m_ReqHFAnchor(0),
    m_pRespHFConfig(pRespHFConfig),
    m_pStatusLine(NULL),
    m_pRespHeaderField(NULL),
    m_State(SR_INITAITED),
    m_SerializeStatus(SERIALIZE_START_LINE),
    m_ProcessRespStatus(PROCESS_RESP_STATUS_LINE)
{
}

CHttpBaseRequest::~CHttpBaseRequest()
{
    delete m_pReqHeaderField;
    delete m_pRespHeaderField;
}

bool CHttpBaseRequest::Serialize(uint8_t* pBuffer, size_t bufLen, size_t* pOutLen)
{
    ASSERT(pOutLen);
    ASSERT(bufLen > 0);
    ASSERT(m_State == SR_INITAITED || m_State == SR_SENDING);

    uint8_t* pCur = pBuffer;
    uint8_t* pEnd = pBuffer + bufLen;
    size_t curPrintLen = 0;
    bool bFinished = false;

    *pOutLen = 0;
    if (m_State == SR_INITAITED) {
        m_State = SR_SENDING;
    }
    switch (m_SerializeStatus) {
    case SERIALIZE_START_LINE:
        bFinished = m_RequestLine.Serialize(
            reinterpret_cast<char*>(pCur), pEnd - pCur, &curPrintLen);
        *pOutLen += curPrintLen;
        pCur += curPrintLen;
        if (!bFinished || pCur == pEnd) {
            OUTPUT_NOTICE_TRACE("Buffer is not enough for fill start line\n");
            return false;
        }
        m_SerializeStatus = SERIALIZE_HEADER_FIELD;
        // Fall through
    case SERIALIZE_HEADER_FIELD:
        m_ReqHFAnchor = m_pReqHeaderField->Serialize(
            reinterpret_cast<char*>(pCur),
            pEnd - pCur,
            ": ", "\r\n",
            m_ReqHFAnchor,
            &curPrintLen);
        *pOutLen += curPrintLen;
        pCur += curPrintLen;
        if (!m_pReqHeaderField->IsEnd(m_ReqHFAnchor) || pCur + 2 > pEnd) {
            return false;
        }
        *pCur++ = '\r';
        *pCur++ = '\n';
        *pOutLen += 2;
        m_SerializeStatus = SERIALIZE_PAYLOAD;
        // Fall Through
    case SERIALIZE_PAYLOAD:
        bFinished = SerializePayload(pCur, pEnd - pCur, &curPrintLen);
        if (bFinished) {
            m_SerializeStatus = SERIALIZE_COMPLETE;
            m_State = SR_RECVING;
        }
        break;
    case SERIALIZE_COMPLETE:
        bFinished = true;
        break;

    default:
        ASSERT(false);
        break;
    }
    return bFinished;
}

ErrorCode CHttpBaseRequest::OnResponse(
    uint8_t* pData, size_t dataLen, size_t* pConsumedLen)
{
    ASSERT(pData);
    ASSERT(dataLen > 0);
    ASSERT(pConsumedLen);
    ASSERT(m_State == SR_RECVING);

    ErrorCode resErr = EC_SUCCESS;
    size_t statusLineLen = 0;
    size_t headerFieldLen = 0;
    size_t payloadLen = 0;
    char* pCur = reinterpret_cast<char*>(pData);
    char* pEnd = pCur + dataLen;

    switch (m_ProcessRespStatus) {
    case PROCESS_RESP_STATUS_LINE:
    {
        ASSERT(m_pStatusLine == NULL);
        CHeaderParser parser(pCur, dataLen, &m_Buffer);
        resErr = parser.CreateStatusLine(&m_pStatusLine);
        statusLineLen = parser.GetConsumedSize();
        if (m_pStatusLine == NULL) {
            break;
        }
        m_ProcessRespStatus = PROCESS_RESP_HEADER_FIELD;
        pCur += statusLineLen;
        if (pCur == pEnd) {
            break;
        }
        // Fall through
    }
    case PROCESS_RESP_HEADER_FIELD:
    {
        CHeaderField* pHeaderField = m_pRespHeaderField;
        if (m_pRespHFConfig && pHeaderField == NULL) {
            pHeaderField = CHeaderField::CreateInstance(m_pRespHFConfig, m_Buffer);
            if (pHeaderField == NULL) {
                resErr = EC_NO_MEMORY;
                break;
            }
            m_pRespHeaderField = pHeaderField;
        }
        CHeaderParser parser(pCur, pEnd - pCur, &m_Buffer);
        resErr = parser.BuildHeaderField(m_pRespHeaderField);
        headerFieldLen = parser.GetConsumedSize();
        pCur += headerFieldLen;
        if (resErr != EC_SUCCESS) {
            break;
        }
        m_ProcessRespStatus = PROCESS_RESP_PAYLOAD;
        resErr = HandleRespHeader(
            m_pStatusLine->GetVersionID(),
            m_pStatusLine->GetStatusCode(),
            m_pStatusLine->GetStatusPhrase(),
            m_pRespHeaderField);
        if (resErr != EC_SUCCESS) {
            break;
        }
        // Fall through
    }
    case PROCESS_RESP_PAYLOAD:
        resErr = ReceiveRespPayload(
            reinterpret_cast<uint8_t*>(pCur), pEnd - pCur, &payloadLen);
        if (resErr == EC_SUCCESS) {
            m_State = SR_COMPLETE;
        }
        break;
    default:
        ASSERT(false);
        break;
    }

    *pConsumedLen = statusLineLen + headerFieldLen + payloadLen;
    return resErr;
}

void CHttpBaseRequest::OnReset()
{
    m_State = SR_INITAITED;
    m_SerializeStatus = SERIALIZE_START_LINE;
    m_ProcessRespStatus = PROCESS_RESP_STATUS_LINE;
    m_ReqHFAnchor = m_pReqHeaderField->AnchorBegin();
}
