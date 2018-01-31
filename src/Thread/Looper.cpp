/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Looper.h"
#include "ITMessage.h"
#include "Condition.h"
#include "Tracker/Trace.h"

bool CMsgSwitch::WriteMessageSync(ITMessage* pMsg)
{
    if (pMsg->SetSync(true) && WriteMessage(pMsg)) {
        pMsg->WaitSink();
        return true;
    }
    return false;
}

CLooper::CLooper(IRunner& runnerObj, CMsgSwitch& switchObj) :
    m_pThread(NULL),
    m_Runner(runnerObj),
    m_Switch(switchObj),
    m_TimerManager(),
    m_bExit(false)
{
}

CLooper::~CLooper()
{
    delete m_pThread;
}

CLooper* CLooper::CreateInstance(
    const char* pName, IRunner& runnerObj, CMsgSwitch& switchObj)
{
    CLooper* pInstance = new CLooper(runnerObj, switchObj);
    if (pInstance) {
        pInstance->m_pThread = CThread::CreateInstance(
            pName, Run, pInstance, CThread::PRIORITY_NORMAL);
        if (pInstance->m_pThread) {
            switchObj.SetContext(pInstance);
        } else {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

bool CLooper::StartTimer(
    ITimerContext* pHandle,
    int sessionID,
    unsigned int timeout /* milliseconds */,
    bool bRepeat /* = false */)
{
    ASSERT(timeout > 0);

    if (IsInLoop()) {
        m_TimerManager.AddTimer(pHandle, timeout, bRepeat, sessionID, true);
        return true;
    }

    // Async create timer.
    bool bRes = false;
    CreateTimerData ctData(pHandle, sessionID, timeout, bRepeat);
    ITMessage msg(CMID_START_TIMER);
    if (msg.SetData(&ctData, sizeof(ctData))) {
        bRes = m_Switch.WriteMessage(&msg);
    }
    return bRes;
}

bool CLooper::StopTimer(tTimerID timerID)
{
    ASSERT(timerID != INVALID_TIMER_ID);

    if (IsInLoop()) {
        m_TimerManager.DeleteTimer(timerID, true);
        return true;
    }

    ITMessage msg(CMID_STOP_TIMER);
    msg.SetData(&timerID, sizeof(timerID));
    return m_Switch.WriteMessage(&msg);
}

bool CLooper::Exit()
{
    bool bRes = false;
    if (!IsInLoop()) {
        ITMessage msg(CMID_EXIT);
        if (m_Switch.WriteMessage(&msg)) {
            m_pThread->GetExecResult(); // Wait for thread stopped
            bRes = true;
        }
    } else {
        m_bExit = true;
        bRes = true;
    }
    return bRes;
}

void CLooper::HandleMessage(ITMessage* pMsg)
{
    if (pMsg->MsgID >= USER_MSGID_BEGIN) {
        m_Runner.HandleMessage(pMsg);
    } else {
        CreateTimerData* pCreateTimerData = NULL;
        tTimerID* pTimerID = NULL;
        switch (pMsg->MsgID) {
        case CMID_START_TIMER:
            pCreateTimerData = reinterpret_cast<CreateTimerData*>(pMsg->GetData());
            m_TimerManager.AddTimer(
                pCreateTimerData->pContext,
                pCreateTimerData->Interval,
                pCreateTimerData->bRepeated,
                pCreateTimerData->SessionID,
                true);
            break;
        case CMID_STOP_TIMER:
            pTimerID = reinterpret_cast<tTimerID*>(pMsg->GetData());
            m_TimerManager.DeleteTimer(*pTimerID, true);
            break;
        case CMID_EXIT:
            m_bExit = true;
            break;
        default:
            ASSERT(false, "Unknown Msg ID: %d\n", pMsg->MsgID);
            break;
        }
    }
}

void* CLooper::Run(void* pArg)
{
    ASSERT(pArg);

    int timeout = -1;
    CLooper* pInstance = reinterpret_cast<CLooper*>(pArg);
    while (!pInstance->m_bExit) {
        ITMessage msg;
        if (pInstance->m_Switch.ReadMessage(&msg, timeout)) {
            pInstance->HandleMessage(&msg);
            msg.SignalSourceIfNeeded();
            msg.Destroy();
        }
        timeout = pInstance->HandleTimeEvent();
    }
    return NULL;
}
