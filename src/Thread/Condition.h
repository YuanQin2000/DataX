/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_CONDITION_H__
#define __THREAD_CONDITION_H__

#include <pthread.h>
#include "Common/Typedefs.h"
#include "Lock.h"

struct timespec;
class CCondition
{
public:
    CCondition();
    ~CCondition()
    {
        pthread_cond_destroy(&m_CondVar);
    }

    /**
     * @brief Wait on this condition variable.
     * @param pLockedCS The locked critical section, no locked critical section if null.
     * @param millisec milli-seconds to wait, infinit wait if less than 0.
     * @return true if success, otherwise false.
     * @warning The locked critical section must be same as the one used in Signal()
     */
    bool Wait(CCriticalSection* pLockedCS, int millisec = -1);

    /**
     * @brief Signal the waited thread on this condition variable.
     * @param pLockedCS The locked critical section, no locked critical section if null.
     * @return true if success, otherwise false.
     * @warning The locked critical section must be same as the one used in Wait()
     */
    bool Signal(CCriticalSection* pLockedCS);

    void Reset(CCriticalSection* pLockedCS);

private:
    bool m_bSignaled;
    CCriticalSection m_CS;
    pthread_cond_t m_CondVar;
};

#endif