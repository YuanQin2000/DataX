/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ConnectionRunner.h"
#include "Request.h"
#include "Thread/Looper.h"
#include "Tracker/Trace.h"

CConnectionRunner::CConnectionRunner() :
    m_Connections(),
    m_pPoller(NULL),
    m_pLoop(NULL)
{
}

CConnectionRunner::~CConnectionRunner()
{
    delete m_pPoller;
    delete m_pLoop;
}

CConnectionRunner* CConnectionRunner::CreateInstance(const char* pName)
{
    ASSERT(pName);

    CConnectionRunner* pInstance = new CConnectionRunner();
    if (pInstance) {
        CPoller* pPoller = CPoller::CreateInstance(pInstance);
        if (pPoller) {
            pInstance->m_pLoop = CLooper::CreateInstance(pName, *pPoller, *pPoller);
            pInstance->m_pPoller = pPoller;
        }
        if (pInstance->m_pLoop == NULL) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

void CConnectionRunner::OnMessage(void* pMsg)
{
    ASSERT(pMsg);

    Command* pCmd = reinterpret_cast<Command*>(pMsg);
    ASSERT(pCmd && pCmd->pData);

    switch (pCmd->ID) {
    case CID_ADD_CONNECTION:
        DoAddConnection(reinterpret_cast<CConnection*>(pCmd->pData));
        break;
    case CID_REMOVE_CONNECTION:
        DoRemoveConnection(reinterpret_cast<CConnection*>(pCmd->pData));
        break;
    case CID_PUSH_REQUEST:
        DoPushRequest(
            reinterpret_cast<CRequest*>(pCmd->pData),
            reinterpret_cast<sockaddr*>(pCmd->DataAddress));
        break;
    default:
        ASSERT(false, "Unknown Message: %d\n", pCmd->ID);
        break;
    }
}

bool CConnectionRunner::AddConnection(CConnection* pConn)
{
    ASSERT(pConn);

    if (m_pLoop->IsInLoop()) {
        DoAddConnection(pConn);
        return true;
    }

    Command cmd;
    cmd.ID = CID_ADD_CONNECTION;
    cmd.pData = pConn;
    return m_pPoller->SendExtCommand(&cmd, sizeof(cmd));
}

bool CConnectionRunner::PushRequest(CRequest* pReq, const sockaddr* pTarget)
{
    ASSERT(pReq);
    ASSERT(pTarget);

    if (m_pLoop->IsInLoop()) {
        return DoPushRequest(pReq, pTarget);
    }

    uint8_t cmdBuffer[sizeof(Command) + sizeof(sockaddr)];
    Command* pCmd = reinterpret_cast<Command*>(cmdBuffer);
    pCmd->ID = CID_PUSH_REQUEST;
    pCmd->pData = pReq;
    memcpy(pCmd->DataAddress, pTarget, sizeof(sockaddr));
    return m_pPoller->SendExtCommand(pCmd, sizeof(cmdBuffer));
}

bool CConnectionRunner::RemoveConnection(CConnection* pConn)
{
    ASSERT(pConn);

    if (m_pLoop->IsInLoop()) {
        DoRemoveConnection(pConn);
        return true;
    }

    Command cmd;
    cmd.ID = CID_REMOVE_CONNECTION;
    cmd.pData = pConn;
    return m_pPoller->SendExtCommand(&cmd, sizeof(cmd));
}

void CConnectionRunner::DoRemoveConnection(CConnection* pConn)
{
    CPointer address(pConn->GetPeerAddress(), sizeof(sockaddr));
    map<CPointer, CConnection*>::iterator iter = m_Connections.find(address);
    if (iter != m_Connections.end()) {
        m_Connections.erase(iter);
        delete pConn;
    } else {
        ASSERT(false);
    }
}

bool CConnectionRunner::DoPushRequest(CRequest* pReq, const sockaddr* pTarget)
{
    CConnection* pConn = FindConnection(pTarget);
    if (pConn == NULL) {
        pConn = CConnection::CreateInstance(*this, *m_pPoller, pReq, pTarget);
        if (pConn) {
            if (!DoAddConnection(pConn)) {
                delete pConn;   // pIO has been transferred ownership
                pConn = NULL;
            }
        }
    }

    return pConn ? pConn->PushRequest(pReq) : false;
}

CConnection* CConnectionRunner::FindConnection(const sockaddr* pTarget)
{
    CConnection* pConnection = NULL;
    CPointer address(pTarget, sizeof(sockaddr));
    map<CPointer, CConnection*>::iterator iter = m_Connections.find(address);
    if (iter != m_Connections.end()) {
        pConnection = iter->second;
    }
    return pConnection;
}
