/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Thread.h"
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include "Tracker/Trace.h"

using std::malloc;
using std::free;
using std::memcpy;
using std::strerror;

CLocalStorage CThread::s_LocalStorage = CLocalStorage();

CThread::CThread(
    const char* pName,
    tThreadRoutine routine,
    void* pRoutineParam,
    bool bCancallable,
    void* pUserData) :
    m_InitCS(CCriticalSection::LT_NORMAL),
    m_ID(0),
    m_Routine(routine),
    m_pRoutineParam(pRoutineParam),
    m_pHandlers(NULL),
    m_HandlersCount(0),
    m_pResult(NULL),
    m_bJoined(false),
    m_bCancellable(bCancallable),
    m_ErrorNo(0),
    m_pName(pName),
    m_pUserData(pUserData)
{
    ASSERT(pName);
    ASSERT(routine);
}

CThread::~CThread()
{
    if (!m_bJoined) {
        int res = pthread_join(m_ID, &m_pResult);
        if (res < 0) {
            OUTPUT_ERROR_TRACE("pthread_join: %s\n", strerror(res));
            m_pResult = NULL;
        }
    }
    if (m_pHandlers) {
        free(m_pHandlers);
        m_pHandlers = NULL;
    }
}

bool CThread::Initialize(Priority pri, CleanupHandler* pHandlers, size_t handlersCount)
{
    ASSERT(m_pHandlers == NULL);
    ASSERT(m_HandlersCount == 0);

    if (pHandlers) {
        ASSERT(handlersCount > 0);
        m_pHandlers = reinterpret_cast<CleanupHandler*>(
            malloc(sizeof(CleanupHandler) * handlersCount));
        if (m_pHandlers == NULL) {
            OUTPUT_WARNING_TRACE("malloc: %s\n", strerror(errno));
            return false;
        }
        memcpy(m_pHandlers, pHandlers, sizeof(CleanupHandler) * handlersCount);
        m_HandlersCount = handlersCount;
    }

    pthread_attr_t* pAttr = NULL;
    pthread_attr_t attr;
    if (pri != PRIORITY_NORMAL) {
        struct sched_param schParam;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        switch (pri) {
        case PRIORITY_LOWEST:
            schParam.sched_priority = sched_get_priority_min(SCHED_RR);
            break;
        case PRIORITY_LOW:
            schParam.sched_priority = 
                sched_get_priority_min (SCHED_RR) +
                (sched_get_priority_max(SCHED_RR) -
                sched_get_priority_min(SCHED_RR)) / 4;
            break;
        case PRIORITY_HIGH:
            schParam.sched_priority =
                sched_get_priority_max(SCHED_RR) -
                (sched_get_priority_max(SCHED_RR) -
                sched_get_priority_min(SCHED_RR)) / 4;
            break;
        case PRIORITY_HIGHEST:
            schParam.sched_priority = sched_get_priority_max(SCHED_RR);
            break;
        default:
            ASSERT(false);
        }
        pthread_attr_setschedparam(&attr, &schParam);
        pAttr = &attr;
    }

     m_InitCS.Lock();
    int res = pthread_create(&m_ID, pAttr, CreateRoutine, this);
    m_InitCS.Unlock();
    if (pAttr) {
        pthread_attr_destroy(pAttr);
    }

    if (res != 0) {
        OUTPUT_WARNING_TRACE("pthread_create: %s\n", strerror(res));
    }
    return res == 0;
}

void CThread::JoinThread()
{
    if (!m_bJoined) {
        int res = pthread_join(m_ID, &m_pResult);
        if (res == 0) {
            if (m_pHandlers && m_HandlersCount > 0 && m_pResult == PTHREAD_CANCELED) {
                for (size_t i = 0; i < m_HandlersCount; ++i) {
                    m_pHandlers[i].Handler(m_pHandlers[i].pData);
                }
            }
        } else {
            OUTPUT_ERROR_TRACE("pthread_join: %s\n", strerror(res));
            m_pResult = NULL;
        }
        m_bJoined = true;
    }
}

bool CThread::Cancel()
{
    bool bRes = false;
    if (m_bCancellable) {
        int res = pthread_cancel(m_ID);
        if (res == 0) {
            bRes = true;
        } else {
            OUTPUT_WARNING_TRACE("pthread_cancel: %s\n", strerror(res));
        }
    } else {
        OUTPUT_WARNING_TRACE("Thread is NOT cancellable.\n");
    }
    return bRes;
}

CThread* CThread::CreateInstance(
    const char* pName,
    tThreadRoutine routine,
    void* pRoutineParam,
    Priority priority /* = PRIORITY_NORMAL */,
    bool bCancallable /* = false */,
    CleanupHandler* pHandlers /* = NULL */,
    size_t handlersCount /* = 0 */,
    void* pUserData /* = NULL */)
{
    CThread* pThread = new CThread(pName, routine, pRoutineParam, bCancallable, pUserData);
    if (pThread) {
        if (!pThread->Initialize(priority, pHandlers, handlersCount)) {
            delete pThread;
            pThread = NULL;
        }
    }
    return pThread;
}

void* CThread::CreateRoutine(void* pArg)
{
    ASSERT(pArg);

    void* pRes = NULL;
    CThread* pThis = reinterpret_cast<CThread*>(pArg);

    pThis->m_InitCS.Lock();
    s_LocalStorage.SetStorageData(pThis);
    if (pThis->m_bCancellable) {
        // This is the default for NPTL
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    } else {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    }

#if 0   // TODO
    for (size_t i = 0; i < pThis->m_HandlersCount; ++i) {
        pthread_cleanup_push(pThis->m_pHandlers[i].Handler, pThis->m_pHandlers[i].pData);
    }
#endif

    pThis->m_InitCS.Unlock();
    pRes = pThis->m_Routine(pThis->m_pRoutineParam);
    return pRes;
}

int CThread::GetErrorNo()
{
    CThread* pCurrent = GetCurrentThread();
    if (pCurrent) {
        return pCurrent->m_ErrorNo;
    }
    return errno;
}

void CThread::SetErrorNo(int error)
{
    CThread* pCurrent = GetCurrentThread();
    if (pCurrent) {
        pCurrent->m_ErrorNo = error;
    } else {
        errno = error;
    }
}
