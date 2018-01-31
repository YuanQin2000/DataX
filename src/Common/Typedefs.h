/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TYPE_DEFINES_H__
#define __TYPE_DEFINES_H__

#include <cstddef>
#include <cstdint>

//using std::size_t;

namespace NSTypedef
{
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
//    typedef unsigned long long int uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
//    typedef long long int int64_t;

typedef bool (*tStringCompareFunc)(const char*, const char*);
typedef int tIOHandle;
typedef int64_t tSecondTick;
};

using namespace NSTypedef;

#define INVALID_IO_HANDLE -1

/**
 * C++ compile will generate 6 functions if you have not defined them.
 * 1) Default Constructor.
 * 2) Copy Constructor.
 * 3) Destructor.
 * 4) Assignment Operator (=)
 * 5) Address Operator (&)
 * 6) Address Operator for const object
 */
#define DISALLOW_DEFAULT_CONSTRUCTOR(TypeName) private: TypeName()
#define DISALLOW_COPY_CONSTRUCTOR(TypeName)    private: TypeName(const TypeName&)
#define DISALLOW_ASSIGN_OPERATOR(TypeName)     private: TypeName& operator=(const TypeName&)

#if defined(_MSC_VER)    /* Visubal C++ */
#define __ALIGN__(N) __declspec(align(N))
#define __PACKED__
#elif defined(__GNUC__)  /* G++ */
#define __ALIGN__(N) __attribute__((aligned(N)))
#define __PACKED__     __attribute__((packed))
#else
#error "NOT SUPPORT COMPILER"
#endif

#endif