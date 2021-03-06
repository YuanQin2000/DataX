/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TRACKER_TIME_H__
#define __TRACKER_TIME_H__
#include <time.h>

struct timespec* GetProcessStartTime();
void GetProcessElapseTime(struct timespec* pNow);

#endif