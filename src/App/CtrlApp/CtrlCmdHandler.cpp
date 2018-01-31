/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CtrlCmdHandler.h"
#include "Common/ErrorNo.h"
#include "CmdLine/CommandLine.h"
#include "CmdLine/LineArguments.h"
#include "IO/IOContext.h"
#include "IO/IOHelper.h"
#include "ClientIf/IPC.h"
#include "ClientIf/SendRecv.h"
#include "ClientIf/CommandTree.h"
#include "ClientIf/SessionManager.h"
#include "Tracker/Trace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

CCtrlCmdHandler::CCtrlCmdHandler() :
    m_pServerIO(NULL), m_pCmdsInfo(NULL), m_SendBuffer{0}
{
}

CCtrlCmdHandler::~CCtrlCmdHandler()
{
    DestroyClient(m_pServerIO);
    delete m_pCmdsInfo;
}

CCtrlCmdHandler* CCtrlCmdHandler::CreateInstance()
{
    CCtrlCmdHandler* pInstance = new CCtrlCmdHandler();
    if (pInstance == NULL) {
        OUTPUT_WARNING_TRACE("new failed.\n");
        goto ERROR_EXIT;
    }

    pInstance->m_pServerIO = CreateClient();
    if (pInstance->m_pServerIO == NULL) {
        OUTPUT_WARNING_TRACE("Create Client failed\n");
        goto ERROR_EXIT;
    }

    pInstance->m_pCmdsInfo = CCommandTree::CreateInstance(*(pInstance->m_pServerIO));
    if (pInstance->m_pCmdsInfo == NULL) {
        OUTPUT_WARNING_TRACE("Create Commands Information failed\n");
        goto ERROR_EXIT;
    }
    return pInstance;

ERROR_EXIT:
    delete pInstance;
    return NULL;
}

bool CCtrlCmdHandler::HandleCommand(CLineArguments* pArgs)
{
    bool bConsumed = false;
    CLineArguments::ArgumentData* pParam = pArgs->GetArgumentAt(0);
    if (pParam->Type != CLineArguments::AT_STRING) {
        return bConsumed;
    }

    NSCliMsg::MsgRequestID reqID = NSCliMsg::GetRequestIDByName(pParam->pString);
    if (reqID == NSCliMsg::MSG_REQUEST_INVALID) {
        return bConsumed;
    }

    bConsumed = true;
    m_bQuit = false;
    pArgs->PopFront();

    uint16_t session = CSessionManager::Instance()->NewSesssion();
    NSCliMsg::Message* pMsg = CreateMessage(session, reqID, pArgs);
    if (pMsg == NULL) {
        return bConsumed;
    }

    // QUENTIN debug
    DumpBytes(pMsg, pMsg->MessageLength());
    if (!NSSendRecv::Send(pMsg, *m_pServerIO)) {
        m_Error = CEC_IO_ERROR;
        return bConsumed;
    }

    m_Error = CEC_LOCAL_ERROR;
    NSCliMsg::Message* pRecvMsg =
        NSSendRecv::Recv(session, *m_pServerIO, m_RecvBuffer, sizeof(m_RecvBuffer));

    tIOHandle hFile = INVALID_IO_HANDLE;
    if (pArgs->GetOutputFileName()) {
        hFile = open(
            pArgs->GetOutputFileName(),
            O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (hFile == INVALID_IO_HANDLE) {
            m_Error = CEC_FILE_ERROR;
            return bConsumed;
        }
    }

    while (pRecvMsg) {
        if (pRecvMsg->Identifier != NSCliMsg::MSG_IDENTIFIER_RESP) {
            m_Error = CEC_MSG_ERROR;
            break;
        }
        m_Error = CEC_OK;
        NSCliMsg::RespPayload* pResp =
            reinterpret_cast<NSCliMsg::RespPayload*>(pRecvMsg->Payload);
        HandleRespMessage(pResp, hFile);
        bool bLast = pResp->IsLast();
        if (bLast) {
            break;
        }
        pRecvMsg = NSSendRecv::Recv(
            session, *m_pServerIO, m_RecvBuffer, sizeof(m_RecvBuffer));
    }
    if (hFile != INVALID_IO_HANDLE) {
        close(hFile);
    }
    return bConsumed;
}

void CCtrlCmdHandler::HandleRespMessage(
    NSCliMsg::RespPayload* pResp, tIOHandle hFile /* = INVALID_IO_HANDLE */)
{
    if (pResp->IsOK()) {
        size_t dataLen = pResp->GetLength() - sizeof(NSCliMsg::RespPayload);
        m_Error = CEC_OK;
        if (hFile != INVALID_IO_HANDLE && dataLen > 0) {
            size_t len = NSIOHelper::Write(hFile, pResp->Data, dataLen);
            if (len != dataLen) {
                OUTPUT_WARNING_TRACE(
                    "Expect to write %d bytes but actually %d bytes done caused by error: %s\n",
                    dataLen, len, strerror(ERROR_CODE));
            }
        }
        return;
    }

    switch (pResp->StatusCode) {
    case NSCliMsg::MSC_COMMAND_NOT_FOUND:
        m_Error = CEC_CMD_NOT_FOUND;
        break;
    case NSCliMsg::MSC_PARAMETER_INVALID:
        m_Error = CEC_PARAM_INVALID;
        break;
    case NSCliMsg::MSC_SERVER_ERROR:
    default:
        m_Error = CEC_SERVER_ERROR;
        break;
    }
    return;
}

NSCliMsg::Message* CCtrlCmdHandler::CreateMessage(
    uint16_t session, NSCliMsg::MsgRequestID reqID, CLineArguments* pArgs)
{
    static const uint8_t* pEnd = m_SendBuffer + sizeof(m_SendBuffer);

    m_Error = CEC_OK;
    if (!NSCliMsg::HasParam(reqID)) {
        if (pArgs->ArgumentsCount() > 1) {
            m_Error = CEC_PARAM_INVALID;
            return NULL;
        }
    }

    NSCliMsg::Message* pMsg = new (m_SendBuffer)
        NSCliMsg::Message(NSCliMsg::MSG_IDENTIFIER_REQ, session);
    if (pArgs->ArgumentsCount() == 0) {
        return pMsg;
    }

    NSCliMsg::ReqPayload* pRequest =
        reinterpret_cast<NSCliMsg::ReqPayload*>(pMsg->Payload);
    size_t dataLength = 0;
    uint8_t* pCur = pRequest->Blocks;
    CCommandTree::tInfoNode* pCurInfo = m_pCmdsInfo->GetRoot();
    for (size_t i = 0; i < pArgs->ArgumentsCount(); ++i) {
        if (pCur >= pEnd) {
            m_Error = CEC_PARAM_TOO_LONG;
            break;
        }
        CLineArguments::ArgumentData* pParam = pArgs->GetArgumentAt(i);
        size_t len = 0;
        switch (pParam->Type) {
        case CLineArguments::AT_STRING:
            len = EncodeString(&pCurInfo, pParam->pString, pCur, pEnd - pCur);
            break;
        case CLineArguments::AT_FILE:
            len = EncodeFile(&pCurInfo, pParam->pString, pCur, pEnd - pCur);
            break;
        default:
            ASSERT(false);
            break;
        }
        if (len == 0 || pCurInfo == NULL) {
            break;
        }
        pCur += len;
        dataLength += len;
    }

    if (m_Error == CEC_OK) {
        new (pMsg->Payload) NSCliMsg::ReqPayload(
            dataLength, reqID, pArgs->ArgumentsCount());
        return pMsg;
    }
    return NULL;
}

size_t CCtrlCmdHandler::EncodeString(
    CCommandTree::tInfoNode** pInfo,
    const char* pString,
    uint8_t* pBuffer,
    size_t len)
{
    m_Error = CEC_PARAM_INVALID;
    CCommandTree::tInfoNode* pNode = m_pCmdsInfo->FindFitNode(*pInfo, pString);
    if (pNode == NULL) {
        return 0;
    }
    size_t blockSize = 0;
    CCommandTree::InfoElement* pElem = pNode->GetElement();
    if (pElem->Type == CCommandTree::TYPE_COMMAND) {
        blockSize = sizeof(NSCliMsg::CommandDataBlock);
        if (len < blockSize) {
            m_Error = CEC_PARAM_TOO_LONG;
            return 0;
        }
        CCommandTree::CommandItem* pItem =
            reinterpret_cast<CCommandTree::CommandItem*>(pElem->pItemData);
        ASSERT(pItem);
        new (pBuffer) NSCliMsg::CommandDataBlock(pItem->CmdID);
    } else {
        CCommandTree::VariableItem* pItem =
            reinterpret_cast<CCommandTree::VariableItem*>(pElem->pItemData);
        ASSERT(pItem);
        size_t stringLen = strlen(pString);
        uint8_t dataType = NSCliMsg::DT_BYTE_DATA;
        if (pItem->bCharString) {
            ++stringLen;
            dataType = NSCliMsg::DT_CHAR_STRING;
        }
        blockSize = sizeof(NSCliMsg::VariableDataBlock) + stringLen;
        if (len < blockSize) {
            m_Error = CEC_PARAM_TOO_LONG;
            return 0;
        }
        new (pBuffer) NSCliMsg::VariableDataBlock(dataType, stringLen, pString);
    }
    m_Error = CEC_OK;
    *pInfo = pNode;
    return blockSize;
}

size_t CCtrlCmdHandler::EncodeFile(
    CCommandTree::tInfoNode** pInfo,
    const char* pString,
    uint8_t* pBuffer,
    size_t len)
{
    if (len <= sizeof(NSCliMsg::VariableDataBlock)) {
        m_Error = CEC_PARAM_TOO_LONG;
        return 0;
    }
    CCommandTree::tInfoNode* pNode = m_pCmdsInfo->FindVariableNode(*pInfo);
    if (pNode == NULL) {
        m_Error = CEC_PARAM_INVALID;
        return 0;
    }

    m_Error = CEC_FILE_ERROR;
    FILE* pFile = fopen(pString, "r");
    if (pFile == NULL) {
        return 0;
    }

    size_t readBytes = 0;
    size_t freeSize = len - sizeof(NSCliMsg::VariableDataBlock);
    size_t fileSize = NSIOHelper::GetFileSize(pFile);
    uint8_t* pCur = pBuffer + sizeof(NSCliMsg::VariableDataBlock);
    if (fileSize > 0 && fileSize < freeSize &&
        (readBytes = fread(pCur, freeSize, 1, pFile)) == fileSize) {
        *pInfo = pNode;
        CCommandTree::InfoElement* pElem = pNode->GetElement();
        CCommandTree::VariableItem* pParamInfo =
            reinterpret_cast<CCommandTree::VariableItem*>(pElem->pItemData);
        uint8_t dataType = NSCliMsg::DT_BYTE_DATA;
        if (pParamInfo->bCharString) {
            dataType = NSCliMsg::DT_CHAR_STRING;
            pBuffer[fileSize] = '\0';
            ++fileSize;
        }
        new (pBuffer) NSCliMsg::VariableDataBlock(dataType, fileSize);
        m_Error = CEC_OK;
    }
    fclose(pFile);
    return readBytes;
}
