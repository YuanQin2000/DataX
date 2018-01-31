/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ServerIO.h"
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ClientIf/IPC.h"
#include "IO/UDSStream.h"
#include "IO/IOHelper.h"
#include "Tracker/Trace.h"

using std::strerror;

CServerIO::CServerIO(
    tIOHandle io, IServiceHandle& service, size_t maxClients) :
    CPollClient(io, EPOLLIN),
    m_Service(service),
    m_MaxClients(maxClients)
{
    ASSERT(io != INVALID_IO_HANDLE);
}

CServerIO::~CServerIO()
{
    close(PollIO());
    unlink(g_pSocketAddress);
}

CServerIO* CServerIO::CreateInstance(IServiceHandle& service, size_t maxClients)
{
    ASSERT(maxClients > 0);

    CServerIO* pInstance = NULL;
    socklen_t len = 0;
    sockaddr_un unAddress;
    tIOHandle sock = socket(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock == INVALID_IO_HANDLE) {
        OUTPUT_ERROR_TRACE("Open: socket: %s\n", strerror(errno));
        goto FAILED_EXIT;
    }
    unAddress.sun_family = AF_UNIX;
    strncpy(unAddress.sun_path, g_pSocketAddress, sizeof(unAddress.sun_path));
    len = sizeof(unAddress.sun_family) + strlen(unAddress.sun_path);
    unlink(g_pSocketAddress);
    if (bind(sock, (sockaddr *)&unAddress, len) != 0)
    {
        OUTPUT_ERROR_TRACE("bind: %s\n", strerror(errno));
        goto FAILED_EXIT;
    }
    if (listen(sock, maxClients) != 0) {
        OUTPUT_ERROR_TRACE("bind: %s\n", strerror(errno));
        goto FAILED_EXIT;
    }

    pInstance = new CServerIO(sock, service, maxClients);
    if (pInstance == NULL) {
        goto FAILED_EXIT;
    }

    return pInstance;

FAILED_EXIT:
    if (sock != INVALID_IO_HANDLE) {
        close(sock);
    }
    unlink(g_pSocketAddress);
    return NULL;
}

void CServerIO::OnAttached(bool bSuccess)
{
}

void CServerIO::OnDetached()
{
}

void CServerIO::OnIncomingData()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    sockaddr_un address;
    socklen_t len = sizeof(sockaddr_un);
    tIOHandle io = accept(PollIO(), reinterpret_cast<sockaddr*>(&address), &len);
    if (io == INVALID_IO_HANDLE) {
        OUTPUT_ERROR_TRACE("accept: %s\n", strerror(errno));
        return;
    }

    CUDSStreamPeer* pPeer = new CUDSStreamPeer(false, io);
    if (pPeer) {
        if (pPeer->Open()) {
            m_Service.OnClientConnected(pPeer); // Transfer pPeer ownership to service
        } else {
            delete pPeer;
        }
    }
}

void CServerIO::OnOutgoingReady()
{
    ASSERT(false);
}

void CServerIO::OnPeerClosed()
{
}
