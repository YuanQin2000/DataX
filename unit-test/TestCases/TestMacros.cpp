/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Macros.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(CommonMacros)
{

void setup()
{
}

void teardown()
{
}

};

TEST(CommonMacros, ByteOrderSwap)
{
    uint16_t u16NumHost = 0x1234;
    uint16_t u16NumHostRevser = 0x3412;
    uint32_t u32NumHost = 0x12345678;
    uint32_t u32NumHostRevser = 0x78563412;
    uint64_t u64NumHost = 0x123456789ABCDEF1;
    uint64_t u64NumHostRevser = 0xF1DEBC9A78563412;

    uint16_t u16NumLE = HOST2LE16(u16NumHost);
    uint32_t u32NumLE = HOST2LE32(u32NumHost);
    uint64_t u64NumLE = HOST2LE64(u64NumHost);

    uint16_t u16NumBE = HOST2BE16(u16NumHost);
    uint32_t u32NumBE = HOST2BE32(u32NumHost);
    uint64_t u64NumBE = HOST2BE64(u64NumHost);

    if (IsBigEndian) {
        LONGS_EQUAL(u16NumHost, u16NumBE);
        LONGS_EQUAL(u32NumHost, u32NumBE);
        LONGS_EQUAL(u64NumHost, u64NumBE);

        LONGS_EQUAL(u16NumHostRevser, u16NumLE);
        LONGS_EQUAL(u32NumHostRevser, u32NumLE);
        LONGS_EQUAL(u64NumHostRevser, u64NumLE);

        LONGS_EQUAL(u16NumHost, HOST2LE16(u16NumLE));
        LONGS_EQUAL(u32NumHost, HOST2LE32(u32NumLE));
        LONGS_EQUAL(u64NumHost, HOST2LE64(u64NumLE));
    } else {
        LONGS_EQUAL(u16NumHost, u16NumLE);
        LONGS_EQUAL(u32NumHost, u32NumLE);
        LONGS_EQUAL(u64NumHost, u64NumLE);

        LONGS_EQUAL(u16NumHostRevser, u16NumBE);
        LONGS_EQUAL(u32NumHostRevser, u32NumBE);
        LONGS_EQUAL(u64NumHostRevser, u64NumBE);

        LONGS_EQUAL(u16NumHost, HOST2BE16(u16NumBE));
        LONGS_EQUAL(u32NumHost, HOST2BE32(u32NumBE));
        LONGS_EQUAL(u64NumHost, HOST2BE64(u64NumBE));
    }
}