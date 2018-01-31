/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_MACROS_H__
#define __COMMON_MACROS_H__

#include <alloca.h>
#include <errno.h>
#include "Arch.h"

#define OFFSET(type, member) \
    reinterpret_cast<size_t>(&(reinterpret_cast<type*>(0))->member)

#define GET_OBJECT_ADDRESS_BY_MEMBER(type, member, memberAddr) \
    reinterpret_cast<type*>((reinterpret_cast<char*>(memberAddr)) - OFFSET(type, member))

#define VAR_ARRAY(type, len) reinterpret_cast<type*>(alloca(sizeof(type) * (len)))

#define COUNT_OF_ARRAY(array) (sizeof(array) / sizeof(array[0]))

#define SYMBOL_STRING(symbol) #symbol

#define TEST_FLAG(bits, flag) (((bits) & (flag)) != 0)
#define SET_FLAG(bits, flag) (bits) |= (flag)
#define CLEAR_FLAG(bits, flag) (bits) &= ~(flag)

#define EXIT_PROGRAM(n) exit(n)

// Big endian: Big address endian, MSB is on low address
#if __OS_BYTE_ORDER__ == __OS_BIG_ENDIAN__

#define IsBigEndian true

// Host endian -> Big endian (nothing to do)
#define HOST2BE16(n) n
#define HOST2BE32(n) n
#define HOST2BE64(n) n

// Host endian -> Little endian
#define HOST2LE16(n) ((((n) & 0xFF) << 8) | (((n) >> 8) & 0xFF))
#define HOST2LE32(n) ((HOST2BE16((n) & 0xFFFF) << 16) | ((HOST2BE16((n) >> 16)) & 0xFFFF))
#define HOST2LE64(n) ((HOST2BE32((n) & 0xFFFFFFFF) << 32) | ((HOST2BE32((n) >> 32)) & 0xFFFFFFFF))

// Big endian -> Host endian (nothing to do)
#define BE2HOST16(n) n
#define BE2HOST32(n) n
#define BE2HOST64(n) n

// Little endian -> Host endian
#define LE2HOST16(n) HOST2LE16(n)
#define LE2HOST32(n) HOST2LE32(n)
#define LE2HOST64(n) HOST2LE64(n)

#elif __OS_BYTE_ORDER__ == __OS_LITTLE_ENDIAN__   // Host is little endian

#define IsBigEndian false

// Host endian -> Big endian
#define HOST2BE16(n) ((((n) & 0xFF) << 8) | (((n) >> 8) & 0xFF))
#define HOST2BE32(n) ((HOST2BE16((n) & 0xFFFF) << 16) | ((HOST2BE16((n) >> 16)) & 0xFFFF))
#define HOST2BE64(n) ((HOST2BE32((n) & 0xFFFFFFFF) << 32) | ((HOST2BE32((n) >> 32)) & 0xFFFFFFFF))

// Host endian -> Little endian (nothing to do)
#define HOST2LE16(n) n
#define HOST2LE32(n) n
#define HOST2LE64(n) n

// Big endian -> Host endian
#define BE2HOST16(n) HOST2BE16(n)
#define BE2HOST32(n) HOST2BE32(n)
#define BE2HOST64(n) HOST2BE64(n)

// Little endian -> Host endian (nothing to do)
#define LE2HOST16(n) n
#define LE2HOST32(n) n
#define LE2HOST64(n) n

#else
#error "NOT support bit order"

#endif  // end __OS_BYTE_ORDER__ == __OS_BIG_ENDIAN__

#endif