/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __SERVICE_TIMER_MANAGER_H__
#define __SERVICE_TIMER_MANAGER_H__

#include "Common/Table.h"

#define INVALID_TIMER_ID ((tTimerID)-1)

typedef int tTimerID;

class ITimerContext
{
public:
    virtual void OnTimeout(tTimerID timerID) = 0;
    virtual void OnTimeCreated(int sessionID, tTimerID timerID) {}
    virtual void OnTimerDeleted(tTimerID timerID) {}
};

class CTimerManager
{
public:
    CTimerManager();
    ~CTimerManager();

public:
    tTimerID AddTimer(
        ITimerContext* pContext,
        unsigned int millisec,
        bool bRepeat,
        uint16_t sessionID = 0,
        bool bNotify = false);
    void DeleteTimer(tTimerID timerID, bool bNotify = false);
    int RefreshTimer();

    static void Accumulate(struct timespec* pRef, unsigned int millisec);

    static void GetAbsoluteTime(unsigned int millisecLater, struct timespec* pOut)
    {
        ASSERT(pOut);

        clock_gettime(CLOCK_MONOTONIC, pOut);
        CTimerManager::Accumulate(pOut, millisecLater);
    }

    static unsigned int Convert2Millisec(struct timespec* pRef)
    {
        return pRef->tv_sec * 1000 + pRef->tv_nsec / 1000000LL;
    }

private:
    struct TimerItem {
        tTimerID ID;
        uint64_t Timeout;  // Milliseconds, happen time.
        unsigned int Interval;
        ITimerContext* pHandle;
        TimerItem* pNext;
        TimerItem* pPrev;
    };

    void InsertTimer(TimerItem* pTimer);

private:
    CTable m_TimersData;
    TimerItem* m_pRunningTimers;
};

#endif