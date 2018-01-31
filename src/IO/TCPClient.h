/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_TCP_CLIENT_H__
#define __IO_TCP_CLIENT_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "Common/Typedefs.h"
#include "IOContext.h"

class CTcpClient : public CIOContext
{
public:
    CTcpClient(const sockaddr* pPeerAddr, bool bBlocked);
    ~CTcpClient();

    // From CIOContext
    bool Open();

    const sockaddr* PeerAddress() const { return &m_PeerAddress; }

private:
    bool IsHandshakeSuccess();

private:
    sockaddr m_PeerAddress;

    DISALLOW_COPY_CONSTRUCTOR(CTcpClient);
    DISALLOW_ASSIGN_OPERATOR(CTcpClient);
    DISALLOW_DEFAULT_CONSTRUCTOR(CTcpClient);
};

#endif
