/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "LocalStorage.h"
#include <cstring>
#include "Tracker/Trace.h"

CLocalStorage::CLocalStorage()
{
    int res = pthread_key_create(&m_Key, NULL);
    if (res != 0) {
        OUTPUT_ERROR_TRACE("pthread_key_create: %s\n", strerror(res));
        ASSERT(false);
        return;
    }
    SetStorageData(NULL);
}

CLocalStorage::~CLocalStorage()
{
    pthread_key_delete(m_Key);
}

void CLocalStorage::SetStorageData(void* pData)
{
    int res = pthread_setspecific(m_Key, pData);
    if (res != 0) {
        OUTPUT_ERROR_TRACE("pthread_setspecific: %s\n", strerror(res));
        ASSERT(false);
        return;
    }
}
