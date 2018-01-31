/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpCmdHelper.h"
#include <cstring>
#include "Common/Typedefs.h"
#include "Common/ErrorNo.h"
#include "Common/Macros.h"
#include "Common/StringIndex.h"
#include "ClientIf/CliMsg.h"
#include "IO/IOHelper.h"
#include "HTTP/Http.h"
#include "Tracker/Trace.h"

#ifdef __DEBUG__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

using std::strlen;

CHttpCommandHelper::CHttpClient::CHttpClient(
uint16_t session, IResultHandler& handler) :
    m_SessionID(session),
    m_ResultHandler(handler),
    m_pRequest(NULL),
    m_DataLength(0),
    m_DataCount(0)
{
#ifdef __DEBUG__
    m_hDumpFile = open(
        "http-dump.html",
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ASSERT(m_hDumpFile != INVALID_IO_HANDLE);
#endif
}

CHttpCommandHelper::CHttpClient::~CHttpClient()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    delete m_pRequest;

#ifdef __DEBUG__
    if (m_hDumpFile != INVALID_IO_HANDLE) {
        close(m_hDumpFile);
    }
#endif
}

void CHttpCommandHelper::CHttpClient::OnTerminated(ErrorCode status)
{
    OUTPUT_DEBUG_TRACE(
        "%d times received total %d bytes data, Complete status: %s\n",
        m_DataCount, m_DataLength, GetErrorPhrase(status));
    m_ResultHandler.OnResult(m_SessionID, NSCliMsg::MSC_OK);
    delete this;
}

void CHttpCommandHelper::CHttpClient::OnData(uint8_t* pData, size_t len)
{
    ++m_DataCount;
    m_DataLength += len;
    m_ResultHandler.OnResult(m_SessionID, NSCliMsg::MSC_OK, pData, len, false);

#ifdef __DEBUG__
    if (m_hDumpFile != INVALID_IO_HANDLE) {
        size_t wroteLen = NSIOHelper::Write(m_hDumpFile, pData, len);
        if (wroteLen != len) {
            OUTPUT_WARNING_TRACE(
                "Expect to write %d bytes but actually %d bytes done caused by error: %s\n",
                len, wroteLen, strerror(ERROR_CODE));
        }
    }
#endif
}

void CHttpCommandHelper::CHttpClient::OnHttpStatus(
    int code, const char* pStatusPhrase, const tStringMap* pField)
{
    OUTPUT_DEBUG_TRACE("\n\tStatus Code: %d\n\tStatus Phrase: %s\n", code, pStatusPhrase);
    if (pField) {
        tStringMap::const_iterator iter = pField->begin();
        tStringMap::const_iterator iterEnd = pField->end();
        OUTPUT_DEBUG_TRACE("Extension Header Field:\n");
        while (iter != iterEnd) {
            OUTPUT_DEBUG_TRACE("\t%s: %s\n", iter->first, iter->second);
            ++iter;
        }
    }
}

void CHttpCommandHelper::CHttpClient::OnHttpIndication(
    HttpIndication ind, void* pData /* = NULL */)
{
    OUTPUT_DEBUG_TRACE("\n\tHTTP Event: %s\n", HttpIndicationName(ind));
    RedirectData* pRedirect = reinterpret_cast<RedirectData*>(pData);
    switch (ind) {
    case IND_HTTP_URI_CREATED:
    case IND_HTTP_URI_UPDATED:
        OUTPUT_DEBUG_TRACE("New URI is %s\n", pData);
        break;
    case IND_HTTP_URI_REDIRECT:
        OUTPUT_DEBUG_TRACE(
            "%s Redirect URI is %s\n",
            pRedirect->bPermanently ? "Permanent" : "Temporary",
            pRedirect->pLocation);
        break;
    default:
        break;
    }
}


CCommandTree::tInfoNode* CHttpCommandHelper::s_pRoot = CreateRoot();
const char* CHttpCommandHelper::s_pName = "HTTP";

CHttpCommandHelper::CommandData CHttpCommandHelper::s_Commands[CID_COUNT] = {
    { CID_GET, "GET", HandleGetCommand, CreateGetHint },
    { CID_HEAD, "HEAD", HandleHeadCommand, CreateHeaderHint },
    { CID_POST, "POST", HandlePostCommand, CreatePostHint },
    { CID_PUT, "PUT", HandlePutCommand, CreatePutHint },
    { CID_DELETE, "DELETE", HandleDeleteInstance, CreateDeleteHint },
    { CID_CONNECT, "CONNECT", HandleConnectCommand, CreateConnectHint },
    { CID_TRACE, "TRACE", HandleTraceCommand, CreateTraceHint },
    { CID_OPTIONS, "OPTIONS", HandleOptionsCommand, CreateOptionsHint }
};

uint8_t CHttpCommandHelper::s_TreeNodeBuffer[1024] = {0};
uint8_t* CHttpCommandHelper::s_pBufferEnd = s_TreeNodeBuffer + sizeof(s_TreeNodeBuffer);
uint8_t* CHttpCommandHelper::s_pFreeBuffer = s_TreeNodeBuffer;

CHttpCommandHelper::CHttpCommandHelper()
{
}

CHttpCommandHelper::~CHttpCommandHelper()
{
}

const char* CHttpCommandHelper::GetCommandName() const
{
    return s_pName;
}

CCommandTree::tInfoNode* CHttpCommandHelper::GetHint()
{
    return s_pRoot;
}

void CHttpCommandHelper::ExecuteCommand(
    uint16_t sessionID,
    CVector& cmdParam,
    IResultHandler& resultHandler)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    NSCliMsg::MsgStatusCode result = NSCliMsg::MSC_COMMAND_NOT_FOUND;
    tHttpCmdHandler cmdHandler = FindHandler(cmdParam);
    CHttpClient* pUser = NULL;
    if (cmdHandler) {
        pUser = new CHttpClient(sessionID, resultHandler);
        if (pUser) {
            cmdParam.PopFront();
            result = cmdHandler(*pUser, cmdParam);
        } else {
            result = NSCliMsg::MSC_SERVER_ERROR;
        }
    }

    if (result != NSCliMsg::MSC_OK) {
        delete pUser;
        resultHandler.OnResult(sessionID, result);
    }
}

CHttpCommandHelper::tHttpCmdHandler
CHttpCommandHelper::FindHandler(CVector& cmdParam)
{
    tHttpCmdHandler handler = NULL;
    NSCliMsg::CommandDataBlock* pCommand =
        reinterpret_cast<NSCliMsg::CommandDataBlock*>(cmdParam.At(0));
    if (pCommand && pCommand->Type == NSCliMsg::BT_COMMAND) {
        uint8_t id = pCommand->CmdID;
        if (IsValidCommand(id)) {
            handler = s_Commands[id].Handler;
        }
    } else {
        NSCliMsg::DumpBlock(pCommand);
    }
    return handler;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateRoot()
{
    static CCommandTree::InfoElement s_Elem(CCommandTree::TYPE_COMMAND, 0, NULL);
    static CCommandTree::tInfoNode s_Root(&s_Elem);

    size_t nameLen = strlen(s_pName) + 1;
    size_t requiredSize = sizeof(CCommandTree::CommandItem) + nameLen;
    ASSERT(s_pFreeBuffer + requiredSize <= s_pBufferEnd);
    CCommandTree::CommandItem* pInfo =
        reinterpret_cast<CCommandTree::CommandItem*>(s_pFreeBuffer);
    pInfo->CmdID = 0;
    memcpy(pInfo->Name, s_pName, nameLen);
    s_Elem.pItemData = pInfo;
    s_pFreeBuffer += requiredSize;

    size_t count = 0;
    for (size_t i = 0; i < COUNT_OF_ARRAY(s_Commands); ++i) {
        if (s_Commands[i].HintCreator) {
            CCommandTree::tInfoNode* pNode = s_Commands[i].HintCreator();
            if (pNode) {
                ++count;
                s_Root.AddChild(pNode);
            }
        }
    }
    if (count > 0) {
        s_Elem.SubCount = count;
        return &s_Root;
    }
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateGetHint()
{
    static CCommandTree::InfoElement s_ElemCmd(CCommandTree::TYPE_COMMAND, 1, NULL);
    static CCommandTree::InfoElement s_ElemVar(CCommandTree::TYPE_VARIABLE, 0, NULL);
    static CCommandTree::tInfoNode s_Root(&s_ElemCmd);
    static CCommandTree::tInfoNode s_Child(&s_ElemVar);

    const char* s_CmdName = "GET";
    const char* s_VarName = "URI";
    size_t cmdNameLen = strlen(s_CmdName) + 1;
    size_t varNameLen = strlen(s_VarName) + 1;
    size_t requiredSize = sizeof(CCommandTree::CommandItem) + cmdNameLen +
                          sizeof(CCommandTree::VariableItem) + varNameLen;
    ASSERT(s_pFreeBuffer + requiredSize <= s_pBufferEnd);

    CCommandTree::CommandItem* pCmdInfo =
        reinterpret_cast<CCommandTree::CommandItem*>(s_pFreeBuffer);
    CCommandTree::VariableItem* pVarInfo =
        reinterpret_cast<CCommandTree::VariableItem*>(s_pFreeBuffer +
        sizeof(CCommandTree::CommandItem) + cmdNameLen);
    pCmdInfo->CmdID = CID_GET;
    memcpy(pCmdInfo->Name, s_CmdName, cmdNameLen);
    pVarInfo->bCharString = true;
    pVarInfo->bMandatory = true;
    memcpy(pVarInfo->Name, s_VarName, varNameLen);
    s_ElemCmd.pItemData = pCmdInfo;
    s_ElemVar.pItemData = pVarInfo;
    s_pFreeBuffer += requiredSize;
    s_Root.AddChild(&s_Child);
    return &s_Root;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateHeaderHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreatePostHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreatePutHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateDeleteHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateConnectHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateTraceHint()
{
    return NULL;
}

CCommandTree::tInfoNode* CHttpCommandHelper::CreateOptionsHint()
{
    return NULL;
}

// GET: <URL>
NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleGetCommand(CHttpClient& user, CVector& cmdParam)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    if (cmdParam.Count() != 1) {
        return NSCliMsg::MSC_PARAMETER_INVALID;
    }
    NSCliMsg::VariableDataBlock* pBlock =
        reinterpret_cast<NSCliMsg::VariableDataBlock*>(cmdParam.At(0));
    if (pBlock->Type != NSCliMsg::BT_VARIABLE ||
        pBlock->DataType != NSCliMsg::DT_CHAR_STRING) {
        return NSCliMsg::MSC_PARAMETER_INVALID;
    }
    char* pURI = reinterpret_cast<char*>(pBlock->Data);
    if (pURI == NULL) {
        return NSCliMsg::MSC_PARAMETER_INVALID;
    }

    CHttpRequest* pRequest = CHttp::CreateGetRequest(user, pURI);
    if (pRequest && pRequest->Start()) {
        user.SetRequest(pRequest);
        return NSCliMsg::MSC_OK;
    }
    return NSCliMsg::MSC_SERVER_ERROR;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleHeadCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandlePostCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandlePutCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleDeleteInstance(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleConnectCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleTraceCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}

NSCliMsg::MsgStatusCode
CHttpCommandHelper::HandleOptionsCommand(CHttpClient& user, CVector& cmdParam)
{
    return NSCliMsg::MSC_COMMAND_NOT_FOUND;
}


static const CCliCmdHelperRegister g_HttpCmdHelperReg(CHttpCommandHelper::Instance());
