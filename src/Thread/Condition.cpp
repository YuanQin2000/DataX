/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Condition.h"
#include <time.h>
#include <cstring>
#include <errno.h>
#include "Lock.h"
#include "TimerManager.h"
#include "Tracker/Trace.h"

using std::strerror;

CCondition::CCondition() :
    m_bSignaled(false),
    m_CS()
{
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_CondVar, &attr);
    pthread_condattr_destroy(&attr);
}

bool CCondition::Wait(CCriticalSection* pLockedCS, int millisec /* = -1 */)
{
    CCriticalSection* pCS = pLockedCS;
    if (pLockedCS == NULL) {
        pCS = &m_CS;
        m_CS.Lock();
    }
    int res = 0;
    if (!m_bSignaled) {
        if (millisec < 0) {
            // This will cause the mutex is unloked while the lock depth keep same.
            res = pthread_cond_wait(&m_CondVar, pCS->GetMutex());
            ASSERT(m_bSignaled);
        } else if (millisec > 0) {
            struct timespec absTime;
            CTimerManager::GetAbsoluteTime(millisec, &absTime);
            res = pthread_cond_timedwait(&m_CondVar, pCS->GetMutex(), &absTime);
        }
    }
    m_bSignaled = false;
    if (pLockedCS == NULL) {
        m_CS.Unlock();
    }
    bool bRes = (res == 0 || res == ETIMEDOUT);
    if (!bRes) {
        OUTPUT_WARNING_TRACE("pthread_cond_wait: %s\n", strerror(res));
    }
    return bRes;
}

bool CCondition::Signal(CCriticalSection* pLockedCS)
{
    int res = 0;
    if (pLockedCS == NULL) {
        m_CS.Lock();
    }

    if (!m_bSignaled) {
        m_bSignaled = true;
        res = pthread_cond_signal(&m_CondVar);
        if (res != 0) {
            OUTPUT_WARNING_TRACE("pthread_cond_signal: %s\n", strerror(res));
        }
    }

    if (pLockedCS == NULL) {
       m_CS.Unlock();
    }

    return res == 0;
}

void CCondition::Reset(CCriticalSection* pLockedCS)
{
    if (pLockedCS == NULL) {
        m_CS.Lock();
        m_bSignaled = false;
        m_CS.Unlock();
    } else {
        m_bSignaled = false;
    }
}
