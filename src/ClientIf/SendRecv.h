/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_IF_SENDRECV_H__
#define __CLIENT_IF_SENDRECV_H__

#include "CliMsg.h"

class CIOContext;
namespace NSSendRecv
{
    bool Send(NSCliMsg::Message* pMsg, CIOContext& io);
    NSCliMsg::Message* Recv(uint16_t session, CIOContext& io, int timeout = -1);
    NSCliMsg::Message* Recv(uint16_t session, CIOContext& io, uint8_t* pBuf, size_t len, int timeout = -1);
    NSCliMsg::Message* SendAndRecv(
        NSCliMsg::Message* pMsg,
        CIOContext& io,
        uint8_t* pBuf = NULL,
        size_t len = 0,
        int timeout = -1);
};

#endif