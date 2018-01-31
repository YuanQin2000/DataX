/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_LOCAL_STORAGE_H__
#define __THREAD_LOCAL_STORAGE_H__

#include <pthread.h>

/**
 * @brief Thread Local Storage, explicit usage (POSIX)
 * @see Implicit thread local storage:
 *      GCC keyword: __thread
 *          e.g. __thread int tls_number;
 *      Microsoft keyword: __declspec(thread)
 *          e.g. __declspec(thread) int tls_number;
 */

class CLocalStorage
{
public:
    CLocalStorage();
    ~CLocalStorage();

    void* GetStorageData() const
    { return pthread_getspecific(m_Key); }

    void  SetStorageData(void* pData);

private:
    pthread_key_t m_Key;
};

#endif