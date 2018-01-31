/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COREAPP_HTTP_COMMAND_HELPER_H__
#define __COREAPP_HTTP_COMMAND_HELPER_H__

#include "Common/Singleton.h"
#include "HTTP/HttpRequest.h"
#include "ServerIf/CliCmdHelper.h"

class CStringIndex;
class CFile;
class CHttpCommandHelper :
    public ICliCommandHelper,
    public CSingleton<CHttpCommandHelper>
{
public:
    class CHttpClient : public CHttpRequest::IClient
    {
    public:
        CHttpClient(uint16_t session, IResultHandler& handler);
        ~CHttpClient();

        void SetRequest(CHttpRequest* pReq)
        {
            ASSERT(pReq);
            ASSERT(m_pRequest == NULL);
            m_pRequest = pReq;
        }

        // From IClient
        void OnTerminated(ErrorCode status);
        void OnData(uint8_t* pData, size_t len);
        void OnHttpStatus(int code, const char* pStatusPhrase, const tStringMap* pExtField);
        void OnHttpIndication(HttpIndication ind, void* pData = NULL);

    private:
        uint16_t m_SessionID;
        IResultHandler& m_ResultHandler;
        CHttpRequest* m_pRequest;
        size_t m_DataLength;
        size_t m_DataCount;

#ifdef __DEBUG__
        tIOHandle m_hDumpFile;
#endif

    };

public:
    ~CHttpCommandHelper();

    // From ICliCommandHelper
    const char* GetCommandName() const;
    CCommandTree::tInfoNode* GetHint();
    void ExecuteCommand(
        uint16_t sessionID, CVector& cmdParam, IResultHandler& resultHandler);

private:
    CHttpCommandHelper();

private:
    typedef NSCliMsg::MsgStatusCode (*tHttpCmdHandler)(CHttpClient& user, CVector& param);
    typedef CCommandTree::tInfoNode* (*tHttpCmdHintCreator)();

    enum CommandID {
       CID_GET = 0,
       CID_HEAD,
       CID_POST,
       CID_PUT,
       CID_DELETE,
       CID_CONNECT,
       CID_TRACE,
       CID_OPTIONS,
       CID_COUNT
    };

    struct CommandData {
        CommandID CmdID;
        const char* pName;
        tHttpCmdHandler Handler;
        tHttpCmdHintCreator HintCreator;
    };

    static bool IsValidCommand(uint8_t id)
    {
        return id >= 0 && id < CID_COUNT;
    }

    static tHttpCmdHandler FindHandler(CVector& cmdParam);
    static CCommandTree::tInfoNode* CreateRoot();

    static CCommandTree::tInfoNode* CreateGetHint();
    static CCommandTree::tInfoNode* CreateHeaderHint();
    static CCommandTree::tInfoNode* CreatePostHint();
    static CCommandTree::tInfoNode* CreatePutHint();
    static CCommandTree::tInfoNode* CreateDeleteHint();
    static CCommandTree::tInfoNode* CreateConnectHint();
    static CCommandTree::tInfoNode* CreateTraceHint();
    static CCommandTree::tInfoNode* CreateOptionsHint();

    static NSCliMsg::MsgStatusCode HandleGetCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandleHeadCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandlePostCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandlePutCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandleDeleteInstance(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandleConnectCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandleTraceCommand(CHttpClient& user, CVector& param);
    static NSCliMsg::MsgStatusCode HandleOptionsCommand(CHttpClient& user, CVector& param);

private:
    static CCommandTree::tInfoNode* s_pRoot;
    static const char* s_pName;
    static CommandData s_Commands[CID_COUNT];
    static uint8_t s_TreeNodeBuffer[1024];
    static uint8_t* s_pBufferEnd;
    static uint8_t* s_pFreeBuffer;

    friend class CSingleton<CHttpCommandHelper>;
};


#endif