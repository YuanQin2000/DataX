/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <sched.h>
#include "Lock.h"
#include "LocalStorage.h"
#include "Common/ForwardList.h"
#include "Memory/MemoryPool.h"
#include "Tracker/Trace.h"

typedef void* (*tThreadRoutine)(void*);

class CThread
{
public:
    enum Priority {
        PRIORITY_LOWEST = 1,
        PRIORITY_LOW,
        PRIORITY_NORMAL,
        PRIORITY_HIGH,
        PRIORITY_HIGHEST
    };

    struct CleanupHandler {
        void (*Handler)(void*);
        void* pData;
    };

public:
    virtual ~CThread();

    static CThread* CreateInstance(
        const char* pName,
        tThreadRoutine routine,
        void* pRoutineParam,
        Priority priority = PRIORITY_NORMAL,
        bool bCancallable = false,
        CleanupHandler* pHandlers = NULL,
        size_t handlersCount = 0,
        void* pUserData = NULL);

    static CThread* GetCurrentThread()
    {
        return reinterpret_cast<CThread*>(s_LocalStorage.GetStorageData());
    }

    static const char* GetCurrentThreadName()
    {
        CThread* pThread = reinterpret_cast<CThread*>(s_LocalStorage.GetStorageData());
        if (pThread) {
            return pThread->m_pName;
        }
        return "Main Thread";
    }

    static int GetErrorNo();
    static void SetErrorNo(int error);

    void* GetUserData() const { return m_pUserData; }
    void* GetExecResult()
    {
        JoinThread();
        return m_pResult;
    }

    bool Cancel();
    void CheckCancel()
    {
        pthread_testcancel();
    }

private:
    CThread(
        const char* pName,
        tThreadRoutine routine,
        void* pRoutineParam,
        bool bCancallable,
        void* pUserData);
    bool Initialize(Priority pri, CleanupHandler* pHandlers, size_t handlersCount);
    void JoinThread();

    static void* CreateRoutine(void* pArg);

private:
    CCriticalSection m_InitCS;
    pthread_t m_ID;
    tThreadRoutine m_Routine;
    void* m_pRoutineParam;
    CleanupHandler* m_pHandlers;
    size_t m_HandlersCount;
    void* m_pResult;
    bool m_bJoined;
    const bool m_bCancellable;
    int m_ErrorNo;
    const char* m_pName;
    void* m_pUserData;

    static CLocalStorage s_LocalStorage;

    DISALLOW_COPY_CONSTRUCTOR(CThread);
    DISALLOW_ASSIGN_OPERATOR(CThread);
    DISALLOW_DEFAULT_CONSTRUCTOR(CThread);
};

#endif