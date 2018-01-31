/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CliMsg.h"
#include <memory>
#include <cstring>
#include <stdio.h>
#include "Common/Macros.h"
#include "Tracker/Trace.h"

using std::strlen;
using std::pair;

namespace NSCliMsg
{

struct MsgRequestDescr {
    MsgRequestID RequestID;
    const char* pName;
    const char* pHelpDocument;
    bool bHasParameters;
};

static MsgRequestDescr g_RequestDescr[] = {
    { MSG_REQUEST_EXEC,  "exec",  "Execute Command",             true  },
    { MSG_REQUEST_PUSH,  "push",  "Push Command to task stack",  true  },
    { MSG_REQUEST_POP,   "pop",   "Pop Command from task stack", false },
    { MSG_REQUEST_CLEAR, "clear", "Clear the task stack",        false },
};


bool SetBlockIndex(ReqPayload* pRequest, CVector* pVector)
{
    size_t count = pRequest->Count;
    if (count == 0) {
        return true;
    }

    bool bError = false;
    uint8_t* pCur = pRequest->Blocks;
    size_t len = 0;
    for (size_t i = 0; i < count; ++i) {
        Block* pBlock = reinterpret_cast<Block*>(pCur);
        switch (pBlock->Type) {
        case BT_COMMAND:
            len = sizeof(CommandDataBlock);
            break;
        case BT_VARIABLE:
            len = sizeof(VariableDataBlock) +
                reinterpret_cast<VariableDataBlock*>(pBlock)->GetVarLength();
            break;
        default:
            OUTPUT_ERROR_TRACE("Invalid Request Payload\n");
            bError = true;
            break;
        }
        pVector->PushBack(pBlock);
        pCur += len;
    }
    return !bError;
}

size_t GetBlockSize(Block* pBlock)
{
    switch (pBlock->Type) {
    case BT_COMMAND:
        return sizeof(CommandDataBlock);
    case BT_VARIABLE:
        return sizeof(VariableDataBlock) +
            reinterpret_cast<VariableDataBlock*>(pBlock)->GetVarLength();
    case BT_COMMAND_INFO:
        return sizeof(CommandInfoBlock) +
            reinterpret_cast<CommandInfoBlock*>(pBlock)->NameLength;
    case BT_VARIABLE_INFO:
        return sizeof(VariableInfoBlock) +
            reinterpret_cast<VariableInfoBlock*>(pBlock)->NameLength;
    case BT_ROOT_INFO:
        return sizeof(RootInfoBlock);
    default:
        ASSERT(false);
        break;
    }
    return 0;
}

Message* DecodeMessage(
    uint8_t* pData, size_t len, size_t* pConsumedSize /* Output */)
{
    ASSERT(pData);
    ASSERT(len > 0);
    ASSERT(pConsumedSize);

    *pConsumedSize = 0;
    if (len < sizeof(Message)) {
        return NULL;
    }

    Message* pMsg = reinterpret_cast<Message*>(pData);
    size_t msgLen = pMsg->MessageLength();
    if (len < msgLen) {
        return NULL;
    }
    *pConsumedSize = msgLen;
    return pMsg;
}

MsgRequestID GetRequestIDByName(const char* pStr)
{
    MsgRequestID reqID = MSG_REQUEST_INVALID;
    for (size_t i = 0; i < COUNT_OF_ARRAY(g_RequestDescr); ++i) {
        if (strcasecmp(pStr, g_RequestDescr[i].pName) == 0) {
            reqID = g_RequestDescr[i].RequestID;
            break;
        }
    }
    return reqID;
}

const char* GetRequestNameByID(MsgRequestID reqID)
{
    ASSERT(IsValidRequestID(reqID));
    return g_RequestDescr[reqID].pName;
}

bool HasParam(MsgRequestID reqID)
{
    ASSERT(IsValidRequestID(reqID));
    return g_RequestDescr[reqID].bHasParameters;
}

const char* GetResultPhrase(MsgStatusCode sc)
{
    static const char* s_StatusPhrases[] = {
        "OK",
        "Command Not Found",
        "Command Parameter Invalid",
        "Server Internal Error",
    };
    return IsValidStatusCode(sc) ? s_StatusPhrases[sc] : NULL;
}


void DumpBlock(Block* pBlock)
{
    switch (pBlock->Type) {
    case BT_COMMAND:
    {
        CommandDataBlock* pCmdData = reinterpret_cast<CommandDataBlock*>(pBlock);
        printf("Type: BT_COMMAND, Command ID: %d\n", pCmdData->CmdID);
        break;
    }
    case BT_VARIABLE:
    {
        static const char* s_DataTypeName[] = {
            "Char String",
            "Byte Data",
        };
        VariableDataBlock* pVarData = reinterpret_cast<VariableDataBlock*>(pBlock);
        if (pVarData->DataType == DT_CHAR_STRING) {
            printf("Type: BT_VARIABLE, Data Type: %s, Data: %s\n",
                s_DataTypeName[pVarData->DataType], pVarData->Data);
        } else {
            printf("Type: BT_VARIABLE, Data Type: %s, Data Length: %d\n",
                s_DataTypeName[pVarData->DataType], pVarData->VarLength);
        }
        break;
    }
    case BT_COMMAND_INFO:
    {
        CommandInfoBlock* pCmdInfoData = reinterpret_cast<CommandInfoBlock*>(pBlock);
        printf("Type: BT_COMMAND_INFO, ID: %d, Sub Count: %d, Name: %s\n",
               pCmdInfoData->CommandID, pCmdInfoData->SubCount, pCmdInfoData->NameString);
    }
    case BT_VARIABLE_INFO:
    {
        VariableInfoBlock* pVarInfoData = reinterpret_cast<VariableInfoBlock*>(pBlock);
        printf("Type: BT_VARIABLE_INFO, Char String: %s, Mandatory: %s, Sub Count: %d, Name: %s\n",
               pVarInfoData->IsCharString() ? "YES" : "NO",
               pVarInfoData->IsMandatory() ? "YES" : "NO",
               pVarInfoData->SubCount, pVarInfoData->NameString);
    }
    case BT_ROOT_INFO:
    {
        RootInfoBlock* pRootInfoData = reinterpret_cast<RootInfoBlock*>(pBlock);
        printf("Type: BT_ROOT_INFO, Sub Count: %d\n", pRootInfoData->SubCount);
    }
    default:
        ASSERT(false);
        break;
    }
}

void DumpBlocks(CVector* pIndex)
{
    for (size_t i = 0; i < pIndex->Count(); ++i) {
        Block* pBlock = reinterpret_cast<Block*>(pIndex->At(i));
        DumpBlock(pBlock);
    }
}


}