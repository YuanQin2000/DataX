/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Lock.h"
#include <errno.h>
#include <cstring>
#include "Tracker/Trace.h"

CCriticalSection::CCriticalSection(LockType type /* = LT_NORMAL */) :
    m_Type(type),
    m_LockDepth(0)
{
    int res = 0;
    pthread_mutexattr_t attr;

    switch (type) {
    case LT_NORMAL:
        res = pthread_mutex_init(&m_hMutex, NULL);
        ASSERT(res == 0);
        break;

    case LT_RECURSIVE:
        res = pthread_mutexattr_init(&attr);
        ASSERT(res == 0);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        res = pthread_mutex_init(&m_hMutex, &attr);
        ASSERT(res == 0);
        pthread_mutexattr_destroy(&attr);
        break;

    default:
        ASSERT(0);
    }
}

bool CCriticalSection::TryLock()
{
    bool bRes = false;
    int res = pthread_mutex_trylock(&m_hMutex);

    if (res == 0) {
        ++m_LockDepth;
        bRes = true;
    } else if (res == EBUSY || res == EAGAIN) {
        ASSERT(m_LockDepth > 0);
    } else {
        OUTPUT_ERROR_TRACE("pthread_mutex_trylock: %s\n", strerror(res));
        ASSERT(0);
    }
    return bRes;
}
