/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMAND_LINE_H__
#define __COMMAND_LINE_H__

#include "Common/Typedefs.h"
#include "Common/Vector.h"
#include "Common/OctetBuffer.h"
#include "IO/PollClient.h"
#include "IO/IOContext.h"
#include "ClientIf/CliMsg.h"
#include "CliService.h"
#include "CliCmdHelper.h"

class CCliHandler :
    public CPollClient,
    public IResultHandler
{
public:
    CCliHandler(CCliService& service, CIOContext* pCxt);
    ~CCliHandler();

    // From CPollClient
    void OnAttached(bool bSuccess);
    void OnDetached();
    void OnIncomingData();
    void OnOutgoingReady();
    void OnPeerClosed();

    // From IResultHandler
    void OnResult(
        uint16_t sessionID,
        NSCliMsg::MsgStatusCode sc,
        uint8_t* pData = NULL,
        size_t len = 0,
        bool bLast = true);

    void HandlePrivateData(int id, void* pData);

private:
    typedef void (*tCliMsgHandler)(
        CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);

    enum PrivateDataID {
        PDID_RESPONSE = 0,
        PDID_COUNT
    };

    struct ResponseData {
        uint16_t SessionID;
        NSCliMsg::RespPayload* pResponse;

        ResponseData(uint16_t session, NSCliMsg::RespPayload* pResp) :
            SessionID(session),
            pResponse(pResp) {}
        ~ResponseData()
        {
            if (pResponse) {
                free(pResponse);
            }
        }
    };

    void HandleCliCommand(NSCliMsg::Message* pMsg);
    void SendRayload(
        uint8_t id,
        uint16_t sessionID,
        NSCliMsg::PayloadBase* pPayload,
        bool bTransferOwnership = false);
    void Send();
    bool ForwardResponse(uint16_t sessionID, NSCliMsg::RespPayload* pResp);

    void ExecCommand(uint16_t sessionID, NSCliMsg::ReqPayload* pReq);
    void WriteMessage(uint16_t sessionID, NSCliMsg::ReqPayload* pReq);
    void PopCommand(uint16_t sessionID);
    void ClearCommand(uint16_t sessionID);

    static void HandleRequest(CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);
    static void HandleResponse(CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);
    static void HandleQuery(CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);
    static void HandleInfo(CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);
    static void HandleEvent(CCliHandler* pThis, uint16_t sessionID, NSCliMsg::PayloadBase* pData);

    static NSCliMsg::RespPayload* CreateResponse(
        NSCliMsg::MsgStatusCode sc, uint8_t* pData, size_t len, bool bLast);

private:
    CCliService& m_Service;
    CIOContext* m_pIO;          // Owned.
    COctetBuffer m_InBuffer;  // Buffer for input (receive)
    COctetBuffer m_OutBuffer; // Buffer for output (send)
    uint8_t* m_pBuffer;       // Buffer for query/...
    CVector m_TasksData;
    void* m_TaskDataBuf[16];
    uint8_t m_InBufMem[1024];
    uint8_t m_OutBufMem[8192];

#ifdef __DEBUG__
    tIOHandle m_hDumpFile;
#endif

    static const tCliMsgHandler s_CliMsgHandlers[];

    DISALLOW_COPY_CONSTRUCTOR(CCliHandler);
    DISALLOW_ASSIGN_OPERATOR(CCliHandler);
    DISALLOW_DEFAULT_CONSTRUCTOR(CCliHandler);
};

#endif
