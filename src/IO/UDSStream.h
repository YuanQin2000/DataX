/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_UNIX_DOMAIN_SOCKET_STREAM_H__
#define __IO_UNIX_DOMAIN_SOCKET_STREAM_H__

#include <sys/un.h>
#include "Tracker/Trace.h"
#include "IOContext.h"

///////////////////////////////////////////////////////////////////////////////
//
// CUDSStreamPeer Class Definitions
//
///////////////////////////////////////////////////////////////////////////////
class CUDSStreamPeer : public CIOContext
{
public:
    CUDSStreamPeer(bool bBlocked, tIOHandle handle) :
        CIOContext(bBlocked, handle) { ASSERT(handle != INVALID_IO_HANDLE); }
    ~CUDSStreamPeer() { Close(); }

    // From CIOContext
    bool Open();

    DISALLOW_COPY_CONSTRUCTOR(CUDSStreamPeer);
    DISALLOW_ASSIGN_OPERATOR(CUDSStreamPeer);
    DISALLOW_DEFAULT_CONSTRUCTOR(CUDSStreamPeer);
};


///////////////////////////////////////////////////////////////////////////////
//
// CUDSStreamClient Class Definitions
//
///////////////////////////////////////////////////////////////////////////////
class CUDSStreamClient : public CIOContext
{
public:
    CUDSStreamClient(const char* pPath, bool bBlocked);
    ~CUDSStreamClient();

    // From CIOContext
    bool Open();

private:
    const char* m_pSocketPath;
    sockaddr_un m_SocketAddress;
    size_t m_AddressLength;

    DISALLOW_COPY_CONSTRUCTOR(CUDSStreamClient);
    DISALLOW_ASSIGN_OPERATOR(CUDSStreamClient);
    DISALLOW_DEFAULT_CONSTRUCTOR(CUDSStreamClient);
};

#endif