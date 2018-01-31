/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATA_COM_CONNECTION_H__
#define __DATA_COM_CONNECTION_H__

#include "Common/Typedefs.h"
#include "Common/DuplexList.h"
#include "Common/ErrorNo.h"
#include "Memory/MemoryPool.h"
#include "Request.h"
#include "IO/IOContext.h"
#include "IO/Poller.h"
#include "IO/PollClient.h"
#include "Tracker/Trace.h"
#include <sys/socket.h>

class CConnectionRunner;
class CConfigure;
class CController;
class CRequest;
class COctetBuffer;
class CConnection : public CPollClient
{
public:
    ~CConnection();

    // From CPollClient
    void OnAttached(bool bSuccess);
    void OnDetached();
    void OnIncomingData();
    void OnOutgoingReady();
    void OnPeerClosed();

    bool ActivateSecure();
    bool TryPopRequest(CRequest* pReq);

    bool PushRequest(CRequest* pReq)
    {
        return m_PendingRequests.PushBack(pReq);
    }

    bool PushInstantRequest(CRequest* pReq)
    {
        return m_PendingRequests.PushFront(pReq);
    }

    const sockaddr* GetPeerAddress() const { return &m_PeerAddress; }

    static CConnection* CreateInstance(
        CConnectionRunner& runContext,
        CPoller& poller,
        CRequest* pRequest,
        const sockaddr* pAddr);

private:
    CConnection(
        CConnectionRunner& runContext,
        CPoller& poller,
        CIOContext* pIO,
        CController* pController,
        const sockaddr* pAddr);
    
    CRequest* PrepareSendRequest();

    void DoSend();
    bool SendData();
    void DoReceive();
    void ReceiveData();

    bool ResetIO();
    void Terminate(ErrorCode ec);
    void HandleLocalError(int err = 0);

    bool IsIdle() const
    {
        return m_PendingRequests.Count() == 0 &&
               m_WaitingRequests.Count() == 0 &&
               m_pSendingRequest == NULL;
    }

    static void ResendRequests(void* pConn);
    static void ReleaseBuffer(void* pBuf);

private:
    CMemoryPool m_MemPool;
    CDuplexList m_PendingRequests;  // Pending on send requests.
    CDuplexList m_WaitingRequests;  // Waiting on the response request.
    CRequest* m_pSendingRequest;    // Not owned.
    CIOContext* m_pIO;   // Owned
    CController* m_pController; // Owned
    CController* m_pIdleController; // Owned
    CConnectionRunner& m_RunContext;
    CPoller& m_Poller;
    ErrorCode m_Error;
    sockaddr m_PeerAddress;

    COctetBuffer* m_pInBuffer;      // Buffer for input (receive), owned
    COctetBuffer* m_pOutBuffer;     // Buffer for output (send), owned

    DISALLOW_DEFAULT_CONSTRUCTOR(CConnection);
    DISALLOW_COPY_CONSTRUCTOR(CConnection);
    DISALLOW_ASSIGN_OPERATOR(CConnection);
};

#endif