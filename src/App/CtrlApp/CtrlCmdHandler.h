/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_SERVER_CMD_HANDLER_H__
#define __CLIENT_SERVER_CMD_HANDLER_H__

#include "CmdLine/CmdHandler.h"
#include "ClientIf/CliMsg.h"
#include "ClientIf/CommandTree.h"

class CIOContext;
class CCtrlCmdHandler : public CCmdHandler
{
public:
    ~CCtrlCmdHandler();

    // From CCmdHandler
    bool HandleCommand(CLineArguments* pArgs);

    static CCtrlCmdHandler* CreateInstance();

protected:
    CCtrlCmdHandler();

private:
    void HandleRespMessage(
        NSCliMsg::RespPayload* pResp, tIOHandle hFile = INVALID_IO_HANDLE);

    NSCliMsg::Message* CreateMessage(
        uint16_t session, NSCliMsg::MsgRequestID reqID, CLineArguments* pArgs);
    size_t EncodeString(
        CCommandTree::tInfoNode** pInfo,
        const char* pString,
        uint8_t* pBuffer,
        size_t len);
    size_t EncodeFile(
        CCommandTree::tInfoNode** pInfo,
        const char* pString,
        uint8_t* pBuffer,
        size_t len);

private:
    CIOContext* m_pServerIO;    // Owned
    CCommandTree* m_pCmdsInfo;  // Owned.
    uint8_t m_SendBuffer[NSCliMsg::MAX_PACKET_LENGTH];
    uint8_t m_RecvBuffer[NSCliMsg::MAX_PACKET_LENGTH];
};

#endif