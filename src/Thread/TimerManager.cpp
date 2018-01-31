/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "TimerManager.h"
#include "Tracker/Trace.h"

CTimerManager::CTimerManager() :
    m_TimersData(8, sizeof(TimerItem)),
    m_pRunningTimers(NULL)
{
}

CTimerManager::~CTimerManager()
{
}

tTimerID CTimerManager::AddTimer(
    ITimerContext* pContext,
    unsigned int millisec,
    bool bRepeat,
    uint16_t sessionID /* = 0 */,
    bool bNotify /* = false */)
{
    ASSERT(millisec > 0);
    ASSERT(pContext);

    tTimerID timerID = INVALID_TIMER_ID;
    CTable::tIndex index = 0;
    TimerItem* pNode = reinterpret_cast<TimerItem*>(m_TimersData.Allocate(&index));
    if (pNode) {
        pNode->ID = static_cast<tTimerID>(index);
        pNode->Timeout = millisec;
        pNode->Interval = bRepeat ? millisec : 0;
        pNode->pHandle = pContext;
        InsertTimer(pNode);
        timerID = static_cast<tTimerID>(index);
    }
    if (bNotify) {
        pContext->OnTimeCreated(sessionID, timerID);
    }
    return timerID;
}

void CTimerManager::DeleteTimer(tTimerID timerID, bool bNotify /* = false */)
{
    ASSERT(timerID != INVALID_TIMER_ID);

    TimerItem* pNode = reinterpret_cast<TimerItem*>(
        m_TimersData.At(static_cast<CTable::tIndex>(timerID)));
    if (pNode == NULL) {
        ASSERT(false);
        return;
    }

    if (pNode->pPrev) {
        pNode->pPrev->pNext = pNode->pNext;
    } else {
        m_pRunningTimers = pNode->pNext;
    }
    if (pNode->pNext) {
        pNode->pNext->pPrev = pNode->pPrev;
    }
    pNode->pHandle->OnTimerDeleted(timerID);
    m_TimersData.Release(static_cast<CTable::tIndex>(timerID));
}

int CTimerManager::RefreshTimer()
{
    int nextTimeout = -1;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t millisec = Convert2Millisec(&now);

    TimerItem* pCur = m_pRunningTimers;
    TimerItem* pRepeatHead = NULL;
    TimerItem* pRepeatTail = NULL;
    while (pCur) {
        if (millisec < pCur->Timeout) {
            break;
        }
        // Timeout
        TimerItem* pTmp = pCur;
        pTmp->pHandle->OnTimeout(pTmp->ID);
        pCur = pCur->pNext;
        pCur->pPrev = NULL;
        clock_gettime(CLOCK_MONOTONIC, &now);
        millisec = Convert2Millisec(&now);
        if (pTmp->Interval == 0) {
            m_TimersData.Release(static_cast<CTable::tIndex>(pTmp->ID));
        } else {
            pTmp->Timeout = millisec + pTmp->Interval;
            pTmp->pPrev = NULL;
            pTmp->pNext = NULL;
            if (pRepeatHead) {
                ASSERT(pRepeatTail);
                pRepeatTail->pNext = pTmp;
                pTmp->pPrev = pRepeatTail;
            } else {
                ASSERT(!pRepeatTail);
                pRepeatHead = pTmp;
                pRepeatTail = pTmp;
            }
        }
    }
    m_pRunningTimers = pCur;
    if (pRepeatTail) {
        ASSERT(pRepeatTail->Timeout <= m_pRunningTimers->Timeout);
        m_pRunningTimers->pPrev = pRepeatTail;
        pRepeatTail->pNext = m_pRunningTimers;
        m_pRunningTimers = pRepeatHead;
    }
    if (m_pRunningTimers) {
        nextTimeout = m_pRunningTimers->Timeout - millisec;
    }
    return nextTimeout;
}

void CTimerManager::InsertTimer(TimerItem* pTimer)
{
    ASSERT(pTimer);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    pTimer->Timeout += Convert2Millisec(&now);
    TimerItem* pPrev = NULL;
    TimerItem* pCur = m_pRunningTimers;
    while (pCur) {
        if (pTimer->Timeout < pCur->Timeout) {
            pTimer->pNext = pCur;
            pTimer->pPrev = pCur->pPrev;
            if (pCur->pPrev) {
                pCur->pPrev->pNext = pTimer;
                pCur->pPrev = pTimer;
            } else {
                m_pRunningTimers = pTimer;
            }
            break;
        }
        pPrev = pCur;
        pCur = pCur->pNext;
    }
    if (!pCur) {
        // Not inserted.
        pTimer->pPrev = pPrev;
        pTimer->pNext = NULL;
        if (pPrev) {
            pPrev->pNext = pTimer;
        } else {
            m_pRunningTimers = pTimer;
        }
    }
}

void CTimerManager::Accumulate(struct timespec* pRef, unsigned int millisec)
{
    ASSERT(pRef);

    if (millisec >= 1000) {
        pRef->tv_sec  += millisec / 1000;
        pRef->tv_nsec += (millisec % 1000) * 1000000LL;
    } else {
        pRef->tv_nsec += millisec * 1000000LL;
    }

    while (pRef->tv_nsec >= 1000000000LL) {
        pRef->tv_nsec -= 1000000000LL;
        ++pRef->tv_sec;
    }
}
