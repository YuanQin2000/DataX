/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <utility>
#include "CliService.h"
#include "CliHandler.h"
#include "ServerIO.h"
#include "Thread/Looper.h"
#include "IO/IOContext.h"
#include "Tracker/Trace.h"

using std::pair;

CCliService::CCliService() :
    m_bRunning(false),
    m_pPoller(NULL),
    m_pLoop(NULL),
    m_CliHandlers(),
    m_CS(),
    m_CondVar()
{
}

CCliService::~CCliService()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    if (m_bRunning) {
        Stop();
    }
    set<CCliHandler*>::iterator iter = m_CliHandlers.begin();
    set<CCliHandler*>::iterator iterEnd = m_CliHandlers.end();
    while (iter != iterEnd) {
        delete *iter;
        ++iter;
    }
    delete m_pLoop;
    delete m_pPoller;
}

void CCliService::OnMessage(void* pMsg)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CliCommand* pCliCmd = reinterpret_cast<CliCommand*>(pMsg);
    switch (pCliCmd->CmdID) {
    case MSGID_CLI_HANDLER:
        pCliCmd->pHandler->HandlePrivateData(pCliCmd->DataID, pCliCmd->pData);
        break;
    default:
        ASSERT(false);
    }
}

void CCliService::OnClientConnected(CIOContext* pClient)
{
    ASSERT(pClient);
    ASSERT(!pClient->IsBlockMode());

    CCliHandler* pCli = new CCliHandler(*this, pClient);
    if (pCli) {
        if (m_pPoller->AddClient(pCli)) {
            pair<set<CCliHandler*>::iterator, bool> res = m_CliHandlers.insert(pCli);
            ASSERT(res.second);
            return;
        }
    }
    delete pClient;
    delete pCli;
}

bool CCliService::Start()
{
    CSectionLock lock(m_CS);

    if (m_bRunning) {
        ASSERT(m_pPoller);
        return true;
    }

    ASSERT(m_pPoller == NULL);
    CPoller* pPoller = CPoller::CreateInstance(this);
    if (pPoller == NULL) {
        return false;
    }
    CLooper* pLoop = CLooper::CreateInstance("CliService", *pPoller, *pPoller);
    if (pLoop == NULL) {
        delete pPoller;
        return false;
    }

    CServerIO* pServerIO = CServerIO::CreateInstance(*this, MAX_NUM_CLIENTS);
    if (pServerIO == NULL) {
        delete pPoller;
        delete pLoop;
        return false;
    }
    if (!pPoller->AddClient(pServerIO, true)) {
        delete pPoller;
        delete pServerIO;
        delete pLoop;
        return false;
    }
    m_bRunning = true;
    m_pPoller = pPoller;
    m_pLoop = pLoop;
    return true;
}

void CCliService::Stop()
{
    CSectionLock lock(m_CS);
    if (m_bRunning) {
        ASSERT(m_pLoop);
        m_pLoop->Exit();
        m_bRunning = false;
    }
    m_CondVar.Signal(&m_CS);
}

void CCliService::WaitStopped()
{
    m_CS.Lock();
    if (!m_bRunning) {
        m_CS.Unlock();
        return;
    }
    m_CondVar.Wait(&m_CS);
    m_CS.Unlock();
}
