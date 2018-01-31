/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __SERVER_IF_SERVER_IO_H__
#define __SERVER_IF_SERVER_IO_H__

#include "Common/Typedefs.h"
#include "IO/PollClient.h"

class CIOContext;
class CServerIO : public CPollClient
{
public:
    class IServiceHandle
    {
    public:
        virtual void OnClientConnected(CIOContext* pClient) = 0;
    };

public:
    CServerIO(tIOHandle io, IServiceHandle& service, size_t maxClients);
    ~CServerIO();

    // From CPollClient
    void OnAttached(bool bSuccess);
    void OnDetached();
    void OnIncomingData();
    void OnOutgoingReady();
    void OnPeerClosed();

    static CServerIO* CreateInstance(IServiceHandle& service, size_t maxClients);

private:
    IServiceHandle& m_Service;
    const size_t m_MaxClients;
};

#endif