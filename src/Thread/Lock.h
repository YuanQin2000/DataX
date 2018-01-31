/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_LOCK_H__
#define __THREAD_LOCK_H__

#include <pthread.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

class CCriticalSection
{
public:
    enum LockType {
        LT_NORMAL,
        LT_RECURSIVE
    };

public:
    CCriticalSection(LockType type = LT_NORMAL);
    ~CCriticalSection()
    {
        ASSERT(m_LockDepth == 0, "object(%#x) m_LockDepth == %d\n", m_LockDepth);
        pthread_mutex_destroy(&m_hMutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&m_hMutex);
        ++m_LockDepth;
    }

    void Unlock()
    {
        ASSERT(m_LockDepth > 0, "(%#x): m_LockDepth is %d\n", this, m_LockDepth);
        --m_LockDepth;
        pthread_mutex_unlock(&m_hMutex);
    }

    bool TryLock();

    LockType Type() const { return m_Type; }
    pthread_mutex_t* GetMutex() { return &m_hMutex; }

private:
    const LockType m_Type;
    int m_LockDepth;
    pthread_mutex_t m_hMutex;

    DISALLOW_COPY_CONSTRUCTOR(CCriticalSection);
    DISALLOW_ASSIGN_OPERATOR(CCriticalSection);
};

class CSectionLock
{
public:
    CSectionLock(CCriticalSection& cs) : m_CS(cs)
    {
        m_CS.Lock();
    }

    ~CSectionLock()
    {
        m_CS.Unlock();
    }

private:
    CCriticalSection& m_CS;

    DISALLOW_DEFAULT_CONSTRUCTOR(CSectionLock);
    DISALLOW_COPY_CONSTRUCTOR(CSectionLock);
    DISALLOW_ASSIGN_OPERATOR(CSectionLock);
};

#endif