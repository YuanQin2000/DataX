/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "UDSStream.h"
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "IOHelper.h"
#include "Tracker/Trace.h"

using std::strerror;


///////////////////////////////////////////////////////////////////////////////
//
// CUDSStreamPeer Class Implementation
//
///////////////////////////////////////////////////////////////////////////////
bool CUDSStreamPeer::Open()
{
    NSIOHelper::SetIOBlockMode(m_hIO, false);
    return m_hIO != INVALID_IO_HANDLE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CUDSStreamClient Class Implementation
//
///////////////////////////////////////////////////////////////////////////////
CUDSStreamClient::CUDSStreamClient(const char* pPath, bool bBlocked) :
    CIOContext(bBlocked),
    m_pSocketPath(pPath)
{
    ASSERT(m_pSocketPath);
    m_SocketAddress.sun_family = AF_UNIX;
    strncpy(m_SocketAddress.sun_path, m_pSocketPath, sizeof(m_SocketAddress.sun_path));
    m_AddressLength = sizeof(m_SocketAddress.sun_family) + strlen(m_SocketAddress.sun_path);
}

CUDSStreamClient::~CUDSStreamClient()
{
    Close();
}

bool CUDSStreamClient::Open()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    if (m_hIO != INVALID_IO_HANDLE) {
        // Already Opened
        return true;
    }

    int type = IsBlockMode() ? SOCK_STREAM : (SOCK_STREAM | SOCK_NONBLOCK);
    m_hIO = socket(PF_UNIX, type, 0);
    if (m_hIO == INVALID_IO_HANDLE) {
        OUTPUT_ERROR_TRACE("Open: socket: %s\n", strerror(errno));
        return false;
    }

    if (connect(m_hIO, (sockaddr *)&m_SocketAddress, m_AddressLength) != 0) {
        OUTPUT_ERROR_TRACE("connect: %s\n", strerror(errno));
        Close();
        return false;
    }
    return true;
}
