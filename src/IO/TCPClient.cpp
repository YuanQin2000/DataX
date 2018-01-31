/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "TCPClient.h"
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "IOHelper.h"
#include "Tracker/Trace.h"

using std::memcpy;
using std::strerror;

// TODO: implement sigaction for SIGURG signal.
CTcpClient::CTcpClient(const sockaddr* pPeerAddr, bool bBlocked) :
    CIOContext(bBlocked)
{
    ASSERT(pPeerAddr);

    memcpy(&m_PeerAddress, pPeerAddr, sizeof(sockaddr));
}

CTcpClient::~CTcpClient()
{
    Close();
}

bool CTcpClient::Open()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    if (m_hIO != INVALID_IO_HANDLE) {
        return true;
    }

    int type = IsBlockMode() ? SOCK_STREAM : (SOCK_STREAM | SOCK_NONBLOCK);
    m_hIO = socket(AF_INET, type, 0);
    if (m_hIO == INVALID_IO_HANDLE) {
        OUTPUT_ERROR_TRACE("Open: socket: %s\n", strerror(errno));
        return false;
    }

    /*
    struct timeval timeout = { CONNECT_TIMEOUT, 0 };
    setsockopt(m_hIO, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));
    */
    if (connect(m_hIO, &m_PeerAddress, sizeof(sockaddr)) == 0 ||
        errno == EINPROGRESS) {
        return true;
    }

    // Error happened
    close(m_hIO);
    m_hIO = INVALID_IO_HANDLE;
    OUTPUT_ERROR_TRACE("Connect: %s\n", strerror(errno));
    return false;
}

bool CTcpClient::IsHandshakeSuccess()
{
    ASSERT(m_hIO != INVALID_IO_HANDLE);

    int err = 0;
    socklen_t len = sizeof(err);
    int res = getsockopt(m_hIO, SOL_SOCKET, SO_ERROR, &err, &len);
    if (res < 0 || err != 0) {
        ///////////////////////////////////////////////////////////////////////
        // TCP handshake error.
        // Solaris return -1 and not set err (but set errno)
        // BSD return 0 and set err (but not set errno)
        ///////////////////////////////////////////////////////////////////////
        if (err == 0) {
            err = errno;
        }
        OUTPUT_ERROR_TRACE("TCP handshake failed (connect): %s\n", strerror(err));
        return false;
    }
    return true;
}
