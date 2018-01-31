/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Time.h"

struct timespec* GetProcessStartTime()
{
    static bool s_bInited = false;
    static struct timespec s_SystemStartTime;

    if (!s_bInited) {
        clock_gettime(CLOCK_MONOTONIC, &s_SystemStartTime);
        s_bInited = true;
    }
    return &s_SystemStartTime;
}

void GetProcessElapseTime(struct timespec* pNow)
{
    static struct timespec* pSystemStartTime = GetProcessStartTime();

    clock_gettime(CLOCK_MONOTONIC, pNow);
    pNow->tv_sec -= pSystemStartTime->tv_sec;
    pNow->tv_nsec -= pSystemStartTime->tv_nsec;
    if (pNow->tv_nsec < 0) {
        pNow->tv_nsec += 1000000000;
        --pNow->tv_sec;
    }
    pNow->tv_nsec /= 1000;   // Only need microsecond;
}
