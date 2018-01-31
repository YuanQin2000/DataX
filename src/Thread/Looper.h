/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_LOOPER_H__
#define __THREAD_LOOPER_H__

#include "Common/Typedefs.h"
#include "Thread.h"
#include "TimerManager.h"
#include "Tracker/Trace.h"

struct ITMessage;

class IRunner
{
public:
    virtual ~IRunner() {}

    /**
     * @brief Handle the message.
     * @param pMsg The message to be handled.
     * @warning This function is response to release the parameter message.
     */
    virtual void HandleMessage(ITMessage* pMsg) = 0;
};

class CLooper;
class CMsgSwitch
{
public:
    virtual ~CMsgSwitch() {}

    /**
     * @brief Consume the message from message pool.
     * @param pOutMsg Message object for fill the result.
     * @param timeout specify the time to wait.
     * @return True if got message in time, false otherwise
     */
    virtual bool ReadMessage(ITMessage* pOutMsg, int timeout = -1) = 0;

    /**
     * @brief Produce the message to message pool.
     * @param pMsg specify the message to write.
     * @return true if write is OK, false otherwise.
     */
    virtual bool WriteMessage(ITMessage* pMsg) = 0;

    bool WriteMessageSync(ITMessage* pMsg);

    void SetContext(CLooper* pContext)
    {
        ASSERT(pContext);
        ASSERT(m_pContext == NULL);
        m_pContext = pContext;
    }

protected:
    CMsgSwitch() : m_pContext(NULL) {}

protected:
    CLooper* m_pContext;  // Not owned
};


class CLooper
{
public:
    ~CLooper();

    bool StartTimer(
        ITimerContext* pHandle,
        int sessionID,
        unsigned int timeout,
        bool bRepeat = false);
    bool StopTimer(tTimerID timerID);
    bool Exit();

    bool IsInLoop()
    {
        return CThread::GetCurrentThread() == m_pThread;
    }

    static CLooper* CreateInstance(
        const char* pName, IRunner& runnerObj, CMsgSwitch& switchObj);

private:
    CLooper(IRunner& runnerObj, CMsgSwitch& switchObj);

    void HandleMessage(ITMessage* pMsg);
    int HandleTimeEvent()
    {
        return m_TimerManager.RefreshTimer();
    }

    static void* Run(void* pArg);

private:
    enum ControlMsgID {
        CMID_START_TIMER = 1,
        CMID_STOP_TIMER,
        CMID_EXIT,
    };

    struct CreateTimerData {
        ITimerContext* pContext;
        int SessionID;
        unsigned int Interval;
        bool bRepeated;

        CreateTimerData(
            ITimerContext* pCtx, int session, unsigned int interval, bool bRepeat) :
            pContext(pCtx), SessionID(session), Interval(interval), bRepeated(bRepeat) {}
    };

    CThread* m_pThread;      // Owned
    IRunner& m_Runner;
    CMsgSwitch& m_Switch;
    const char* m_pName;    // Not owned
    CTimerManager m_TimerManager;
    bool m_bExit;

    DISALLOW_DEFAULT_CONSTRUCTOR(CLooper);
    DISALLOW_COPY_CONSTRUCTOR(CLooper);
    DISALLOW_ASSIGN_OPERATOR(CLooper);
};

#endif