/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_SINGTON_H__
#define __COMMON_SINGTON_H__

#include "Common/Arch.h"
#include "Thread/Lock.h"

template <typename T>
class CSingleton
{
public:
    static T* Instance()
    {
#if __cplusplus >= 201103L

///////////////////////////////////////////////////////////////////////////////
//
// Thread-safe under C++11 standard.
// 6.7 [stmt.dcl] p4:
//      If control enters the declaration concurrently while the variable is being initialized,
// the concurrent execution shall wait for completion of the initialization. 
//
///////////////////////////////////////////////////////////////////////////////

        static T s_Instance;
        return &s_Instance;

#else

        static T* s_pInstance = NULL;
        static T s_Instance;
        static CCriticalSection s_CS;

        if (s_pInstance) {
            return s_pInstance;
        }

        s_CS.Lock();
        if (s_Instance == NULL) {
            // s_Instance has been intialized, so on need barrier
            s_pInstance = &s_Instance;
        }
        s_CS.Unlock();
#endif

    }

protected:
    CSingleton() {}
    virtual ~CSingleton() {}
};


template <typename T>
class CSingleton2
{
public:
    static T* GetInstance()
    {
        static bool s_bInitialized = false;
        static T* s_pInstance = NULL;
        static T s_Instance;
        static CCriticalSection s_CS;

        if (s_bInitialized) {
            return s_pInstance;
        }

        s_CS.Lock();
        if (!s_bInitialized) {
            if (s_Instance.Initialize()) {
                s_pInstance = &s_Instance;
            }

            /**
             * @warning A serious potential bug: instruction out of order
             * Compiler optimization barrier, to make sure the variable must be
             * set to true after the instance initialized.
             * Since we are locked, we only consider the compiler barrier here.
             * @see CPU memory barrier (on SMP)
             */
            OPTIMIZE_BARRIER();

            s_bInitialized = true;
        }
        s_CS.Unlock();
        return s_pInstance;
    }

protected:
    CSingleton2()          {}
    virtual ~CSingleton2() {}

private:
    virtual bool Initialize() = 0;
};

#endif
