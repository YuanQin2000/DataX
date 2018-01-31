/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CliHandler.h"
#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Common/ErrorNo.h"
#include "Common/OctetBuffer.h"
#include "Common/CharHelper.h"
#include "Thread/Thread.h"
#include "IO/IOHelper.h"
#include "Tracker/Trace.h"
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <new>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::malloc;
using std::free;
using std::memcpy;

const CCliHandler::tCliMsgHandler CCliHandler::s_CliMsgHandlers[] = {
    HandleRequest,
    HandleResponse,
    HandleQuery,
    HandleInfo,
    HandleEvent
};

CCliHandler::CCliHandler(CCliService& service, CIOContext* pCxt) :
    CPollClient(pCxt->GetHandle(), EPOLLIN | EPOLLOUT),
    m_Service(service),
    m_pIO(pCxt),
    m_InBuffer(m_InBufMem, sizeof(m_InBufMem)),
    m_OutBuffer(m_OutBufMem, sizeof(m_OutBufMem)),
    m_pBuffer(NULL),
    m_TasksData(m_TaskDataBuf, sizeof(m_TaskDataBuf))
{
    ASSERT(pCxt);
    memset(m_TaskDataBuf, 0, sizeof(m_TaskDataBuf));

#ifdef __DEBUG__
    m_hDumpFile = open(
        "payload-dump.txt",
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ASSERT(m_hDumpFile != INVALID_IO_HANDLE);
#endif
}

CCliHandler::~CCliHandler()
{
    delete m_pIO;

#ifdef __DEBUG__
    if (m_hDumpFile != INVALID_IO_HANDLE) {
        close(m_hDumpFile);
    }
#endif
}

void CCliHandler::OnAttached(bool bSuccess)
{
}

void CCliHandler::OnDetached()
{
}

void CCliHandler::OnIncomingData()
{
    bool bContinued = false;
    m_pIO->SetReadable();
    do {
        size_t freeSize = m_InBuffer.GetFreeBufferSize();
        ASSERT(freeSize > 0);
        size_t dataLength = m_pIO->Read(m_InBuffer.GetFreeBuffer(), freeSize);
        bContinued = false;
        if (dataLength > 0) {
            m_InBuffer.SetPushInLength(dataLength);
            size_t consumed = 0;
            NSCliMsg::Message* pMsg = NSCliMsg::DecodeMessage(
                m_InBuffer.GetData(), m_InBuffer.GetDataLength(), &consumed);
            if (pMsg) {
                m_InBuffer.SetPopOutLength(consumed, true);
                HandleCliCommand(pMsg);
                bContinued = true;
            }
        }
    } while (bContinued);
}

void CCliHandler::OnOutgoingReady()
{
    m_pIO->SetWritable();
    Send();
}

void CCliHandler::OnPeerClosed()
{
}

void CCliHandler::OnResult(
    uint16_t sessionID,
    NSCliMsg::MsgStatusCode sc,
    uint8_t* pData /* = NULL */,
    size_t len /* = 0 */,
    bool bLast /* = true */)
{
    ASSERT((pData && len > 0) || (pData == NULL && len == 0 && bLast));

    static const size_t MAX_RESPONSE_DATA_SIZE =
        NSCliMsg::MAX_PACKET_LENGTH -
        (sizeof(NSCliMsg::Message) + sizeof(NSCliMsg::RespPayload));

    size_t remainLen = len;
    uint8_t* pCur = pData;
    do {
        size_t curLen = 0;
        bool bIsLast = false;
        if (remainLen > MAX_RESPONSE_DATA_SIZE) {
            curLen = MAX_RESPONSE_DATA_SIZE;
        } else {
            curLen = remainLen;
            bIsLast = bLast;
        }
        NSCliMsg::RespPayload* pResp = CreateResponse(sc, pCur, curLen, bIsLast);
        if (pResp == NULL) {
            // TODO: if this is the last response, we should send the last notify
            ASSERT(false);
            break;
        }
        if (!ForwardResponse(sessionID, pResp)) {
            free(pResp);
            break;
        }
        pCur += curLen;
        remainLen -= curLen;
    } while (remainLen > 0);    
}

void CCliHandler::HandlePrivateData(int id, void* pData)
{
    ASSERT(pData);

    ResponseData* pRespData = reinterpret_cast<ResponseData*>(pData);
    switch (id) {
    case PDID_RESPONSE:
        SendRayload(
            NSCliMsg::MSG_IDENTIFIER_RESP,
            pRespData->SessionID,
            pRespData->pResponse);
        delete pRespData;
        break;
    default:
        ASSERT(false);
        break;
    }
}

void CCliHandler::HandleCliCommand(NSCliMsg::Message* pMsg)
{
    uint8_t id = pMsg->Identifier;
    if (!NSCliMsg::IsValidIdentifier(id)) {
        OUTPUT_WARNING_TRACE("Unknown message identifier: %d\n", id);
        return;
    }

    unsigned int index = NSCliMsg::Identifier2Index(id);
    NSCliMsg::PayloadBase* pPayload = NULL;
    if (pMsg->MessageLength() > sizeof(NSCliMsg::Message)) {
        pPayload = reinterpret_cast<NSCliMsg::PayloadBase*>(&pMsg->Payload);
    }
    if (s_CliMsgHandlers[index]) {
        (s_CliMsgHandlers[index])(this, pMsg->GetSessionID(), pPayload);
    }
}

void CCliHandler::SendRayload(
    uint8_t id, uint16_t sessionID,
    NSCliMsg::PayloadBase* pPayload,
    bool bTransferOwnership /* = false */)
{
    ASSERT(NSCliMsg::IsValidIdentifier(id));

    size_t payloadLen = pPayload->GetLength();
    size_t msgLen = payloadLen + sizeof(NSCliMsg::Message);

#ifdef __DEBUG__
    ASSERT(payloadLen >= sizeof(NSCliMsg::RespPayload));
    OUTPUT_DEBUG_TRACE(
        "Thread<%s>: Payload Length: %d\n",
        CThread::GetCurrentThreadName(),
        payloadLen - sizeof(NSCliMsg::RespPayload));
#endif

    if (m_OutBuffer.GetFreeBufferSize() < msgLen) {
        OUTPUT_ERROR_TRACE("Send buffer full. Discard response.\n");
        return;
    }

    NSCliMsg::Message* pMsg =
        new (m_OutBuffer.GetFreeBuffer()) NSCliMsg::Message(id, sessionID);
    memcpy(pMsg->Payload, pPayload, payloadLen);
    m_OutBuffer.SetPushInLength(msgLen);
    Send();
}

void CCliHandler::Send()
{
    size_t len = m_OutBuffer.GetDataLength();
    if (!m_pIO->IsWritable() || len == 0) {
        return;
    }

    size_t writeBytes = m_pIO->Write(m_OutBuffer.GetData(), len);
    CIOContext::IOStatus writeStatus = m_pIO->GetStatus();
    ErrorCode error = EC_SUCCESS;
    switch (writeStatus) {
    case CIOContext::IOS_LOCAL_ERROR:
        error = GetStandardErrorCode(ERROR_CODE);
        OUTPUT_ERROR_TRACE("write failed: %s\n", GetErrorPhrase(error));
        break;
    case CIOContext::IOS_CLOSED:  // These errors should take by upper level.
    default:
        break;
    }

    m_OutBuffer.SetPopOutLength(writeBytes, true);
}

bool CCliHandler::ForwardResponse(uint16_t sessionID, NSCliMsg::RespPayload* pResp)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    bool bRes = false;
    ResponseData* pRespData = new ResponseData(sessionID, pResp);
    if (pRespData) {
        CCliService::CliCommand cmd;
        cmd.CmdID = CCliService::MSGID_CLI_HANDLER;
        cmd.DataID = PDID_RESPONSE;
        cmd.pHandler = this;
        cmd.pData = pRespData;
        if (m_Service.WriteMessage(&cmd)) {
            bRes = true;
        } else {
            OUTPUT_ERROR_TRACE("Send Message failed.\n");
            delete pRespData;
            pRespData = NULL;
        }
    }

#ifdef __DEBUG__
    ASSERT(pRespData);
    ASSERT(pRespData->pResponse == pResp);
    if (pResp->GetLength() > sizeof(NSCliMsg::RespPayload)) {
        size_t dataLen = pResp->GetLength() - sizeof(NSCliMsg::RespPayload);
        OUTPUT_DEBUG_TRACE(
            "Thread<%s>: Payload Length: %d\n",
            CThread::GetCurrentThreadName(), dataLen);
        if (m_hDumpFile != INVALID_IO_HANDLE && dataLen > 0) {        
            size_t len = NSIOHelper::Write(m_hDumpFile, pResp->Data, dataLen);
            if (pResp->IsLast()) {
                close(m_hDumpFile);
                m_hDumpFile = INVALID_IO_HANDLE;
            }
            ASSERT(len == dataLen);
        }
    }
#endif

    return bRes;
}

void CCliHandler::HandleRequest(
    CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData)
{
    ASSERT(pThis);

    if (pData == NULL) {
        OUTPUT_WARNING_TRACE("Invalid Request Payload\n");
        return;
    }
    NSCliMsg::ReqPayload* pReq = reinterpret_cast<NSCliMsg::ReqPayload*>(pData);
    uint8_t reqID = pReq->RequestID;
    if (!NSCliMsg::IsValidRequestID(reqID)) {
        OUTPUT_WARNING_TRACE("Invalid Request ID: %d\n", reqID);
        return;
    }

    switch (reqID) {
    case NSCliMsg::MSG_REQUEST_EXEC:
        pThis->ExecCommand(sessionID, pReq);
        break;
    case NSCliMsg::MSG_REQUEST_PUSH:
        pThis->WriteMessage(sessionID, pReq);
        break;
    case NSCliMsg::MSG_REQUEST_POP:
        pThis->PopCommand(sessionID);
        break;
    case NSCliMsg::MSG_REQUEST_CLEAR:
        pThis->ClearCommand(sessionID);
        break;
    default:
        ASSERT(false);
        break;
    }
}

void CCliHandler::HandleResponse(
    CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData)
{
    OUTPUT_WARNING_TRACE("Received unexpected response.\n");
}

void CCliHandler::HandleQuery(
    CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData)
{
    const CCliCmdHelperManager::tHintMsgVector& respMsgs =
        CCliCmdHelperManager::Instance()->GetHint();
    if (respMsgs.size() == 0) {
        NSCliMsg::InfoPayload info(0, 0);
        pThis->SendRayload(NSCliMsg::MSG_IDENTIFIER_INFO, sessionID, &info);
        return;
    }
    CCliCmdHelperManager::tHintMsgVector::const_iterator iter = respMsgs.begin();
    CCliCmdHelperManager::tHintMsgVector::const_iterator iterEnd = respMsgs.end();
    while (iter != iterEnd) {
        NSCliMsg::InfoPayload* pInfo = *iter;
        pThis->SendRayload(NSCliMsg::MSG_IDENTIFIER_INFO, sessionID, pInfo);
        ++iter;
    }
}

void CCliHandler::HandleInfo(
    CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData)
{
    OUTPUT_WARNING_TRACE("Received unexpected information.\n");
}

void CCliHandler::HandleEvent(
    CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData)
{
    OUTPUT_WARNING_TRACE("Not support.\n");
}

void CCliHandler::ExecCommand(uint16_t sessionID, NSCliMsg::ReqPayload* pReq)
{
    if (pReq == NULL || pReq->Count == 0) {
        NSCliMsg::RespPayload resp(NSCliMsg::MSC_PARAMETER_INVALID);
        SendRayload(NSCliMsg::MSG_IDENTIFIER_RESP, sessionID, &resp);
        return;
    }
    CVector command(pReq->Count);
    if (!NSCliMsg::SetBlockIndex(pReq, &command)) {
        NSCliMsg::RespPayload resp(NSCliMsg::MSC_SERVER_ERROR);
        SendRayload(NSCliMsg::MSG_IDENTIFIER_RESP, sessionID, &resp);
        return;
    }
    CCliCmdHelperManager::Instance()->ExecuteCommand(sessionID, command, *this);
}

void CCliHandler::WriteMessage(uint16_t sessionID, NSCliMsg::ReqPayload* pReq)
{
    NSCliMsg::MsgStatusCode sc = NSCliMsg::MSC_PARAMETER_INVALID;
    size_t payloadLen = pReq->GetLength();
    if (pReq && payloadLen > sizeof(NSCliMsg::ReqPayload)) {
        void* pDupData = malloc(payloadLen);
        if (pDupData) {
            memcpy(pDupData, pReq, payloadLen);
            if (m_TasksData.PushBack(pDupData)) {
                sc = NSCliMsg::MSC_OK;
            } else {
                sc = NSCliMsg::MSC_SERVER_ERROR;
                OUTPUT_ERROR_TRACE("Push task failed\n");
                free(pDupData);
            }
        }
    }
    NSCliMsg::RespPayload resp(sc);
    SendRayload(NSCliMsg::MSG_IDENTIFIER_RESP, sessionID, &resp);
}

void CCliHandler::PopCommand(uint16_t sessionID)
{
    NSCliMsg::MsgStatusCode sc = NSCliMsg::MSC_PARAMETER_INVALID;
    void* pTaskData = m_TasksData.At(0);
    m_TasksData.PopFront();
    sc = NSCliMsg::MSC_OK;
    ASSERT(pTaskData);
    free(pTaskData);
    NSCliMsg::RespPayload resp(sc);
    SendRayload(NSCliMsg::MSG_IDENTIFIER_RESP, sessionID, &resp);
}

void CCliHandler::ClearCommand(uint16_t sessionID)
{
    NSCliMsg::MsgStatusCode sc = NSCliMsg::MSC_PARAMETER_INVALID;
    size_t count = m_TasksData.Count();
    if (count > 0) {
        sc = NSCliMsg::MSC_OK;
        for (size_t i = 0; i < count; ++i) {
            void* pTaskData = m_TasksData.At(i);
            ASSERT(pTaskData);
            free(pTaskData);
        }
        m_TasksData.Clear();
    }
    NSCliMsg::RespPayload resp(sc);
    SendRayload(NSCliMsg::MSG_IDENTIFIER_RESP, sessionID, &resp);
}

NSCliMsg::RespPayload* CCliHandler::CreateResponse(
    NSCliMsg::MsgStatusCode sc, uint8_t* pData, size_t len, bool bLast)
{
    NSCliMsg::RespPayload* pResp = NULL;
    void* pMem = malloc(len + sizeof(NSCliMsg::RespPayload));

    if (pMem) {
        pResp = new (pMem) NSCliMsg::RespPayload(sc, len, pData, bLast);
    }
    return pResp;
}