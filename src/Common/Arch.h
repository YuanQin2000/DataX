/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_ARCH_H__
#define __COMMON_ARCH_H__

#include <endian.h>
#include "Common/Typedefs.h"

#define __OS_WORD_SIZE__ 64


#define __OS_BIG_ENDIAN__ 0
#define __OS_LITTLE_ENDIAN__ 1

#if __BYTE_ORDER__ == __BIG_ENDIAN   // Host is big endian
#define __OS_BYTE_ORDER__ __OS_BIG_ENDIAN__
#elif __BYTE_ORDER__ == __LITTLE_ENDIAN // Host is little endian
#define __OS_BYTE_ORDER__ __OS_LITTLE_ENDIAN__
#else
#error "Unknown bit order"
#endif


#define OPTIMIZE_BARRIER() __asm__ __volatile__("" ::: "memory")

static inline int32_t AtomicAdd(volatile int32_t *pAddValue, int32_t value)
{
    return __sync_add_and_fetch(pAddValue, value);
}

static inline int64_t AtomicAdd64(volatile int64_t *pAddValue, int64_t value)
{
    return __sync_add_and_fetch(pAddValue, value);
}

static inline int32_t AtomicSub(volatile int32_t *pAddValue, int32_t value)
{
    return __sync_sub_and_fetch(pAddValue, value);
}

static inline int64_t AtomicSub64(volatile int64_t *pAddValue, int64_t value)
{
    return __sync_sub_and_fetch(pAddValue, value);
}

static inline int32_t AtomicInc(volatile int32_t *pAddValue)
{
    return __sync_add_and_fetch(pAddValue, 1);
}

static inline int64_t AtomicInc64(volatile int64_t *pAddValue)
{
    return __sync_add_and_fetch(pAddValue, 1);
}

static inline int32_t AtomicDec(volatile int32_t *pSubValue)
{
    return __sync_sub_and_fetch(pSubValue, 1);
}

static inline int64_t AtomicDec64(volatile int64_t *pSubValue)
{
    return __sync_sub_and_fetch(pSubValue, 1);
}

#endif