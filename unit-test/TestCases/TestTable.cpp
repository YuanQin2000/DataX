/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Table.h"
#include "Common/Macros.h"
#include "Tracker/Debug.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"
#include <cstring>

using std::memcmp;

TEST_GROUP(Table)
{
    uint32_t m_NumberTestData[10] = {
        1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009
    };

    struct TestData {
        uint32_t Number;
        const char* pName;
        const char* pValue;
    } m_AllData[10] = {
        { 1, "Item1", "Value1" },
        { 2, "Item2", "Value2" },
        { 3, "Item3", "Value3" },
        { 4, "Item4", "Value4" },
        { 5, "Item5", "Value5" },
        { 6, "Item6", "Value6" },
        { 7, "Item7", "Value7" },
        { 8, "Item8", "Value8" },
        { 9, "Item9", "Value9" },
        { 0, "Item0", "Value0" },
    };

void setup()
{
}

void teardown()
{
}

};

TEST(Table, TestAggregatedData1)
{
    CTable table(4, sizeof(uint32_t));
    CHECK(table.IsAggregated());

    uint32_t* pNumber = reinterpret_cast<uint32_t*>(table.At(0));
    CHECK(pNumber == NULL);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_NumberTestData); ++i) {
        CTable::tIndex index = 1000;
        pNumber = reinterpret_cast<uint32_t*>(table.Allocate(&index));
        CHECK(pNumber != NULL);
        LONGS_EQUAL(i, index);
        *pNumber = m_NumberTestData[i];
    }

    for (CTable::tIndex i = 0; i < COUNT_OF_ARRAY(m_NumberTestData); ++i) {
        pNumber = reinterpret_cast<uint32_t*>(table.At(i));
        CHECK(pNumber != NULL);
        LONGS_EQUAL(m_NumberTestData[i], *pNumber);
    }

    table.Release(5);
    pNumber = reinterpret_cast<uint32_t*>(table.At(5));
    CHECK(pNumber == NULL);
    CTable::tIndex index;
    pNumber = reinterpret_cast<uint32_t*>(table.Allocate(&index));
    CHECK(pNumber != NULL);
    LONGS_EQUAL(5, index);
    table.Set(5, &m_NumberTestData[9]);
    LONGS_EQUAL(m_NumberTestData[9], *pNumber);
}

TEST(Table, TestAggregatedData2)
{
    CTable table(4, sizeof(TestData));
    CHECK(table.IsAggregated());

    TestData* pData = reinterpret_cast<TestData*>(table.At(1));
    CHECK(pData == NULL);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        CTable::tIndex index = 1000;
        pData = reinterpret_cast<TestData*>(table.Allocate(&index));
        CHECK(pData != NULL);
        LONGS_EQUAL(i, index);
        pData->Number = m_AllData[i].Number;
        pData->pName = m_AllData[i].pName;
        pData->pValue = m_AllData[i].pValue;
    }

    for (CTable::tIndex i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        pData = reinterpret_cast<TestData*>(table.At(i));
        CHECK(pData != NULL);
        CHECK(pData != &m_AllData[i]);
        LONGS_EQUAL(0, memcmp(pData, &m_AllData[i], sizeof(TestData)));
    }

    table.Release(5);
    pData = reinterpret_cast<TestData*>(table.At(5));
    CHECK(pData == NULL);
    CTable::tIndex index;
    pData = reinterpret_cast<TestData*>(table.Allocate(&index));
    CHECK(pData != NULL);
    LONGS_EQUAL(5, index);
    table.Set(5, &m_AllData[9]);
    CHECK(pData != &m_AllData[9]);
    LONGS_EQUAL(0, memcmp(pData, &m_AllData[9], sizeof(TestData)));
}

TEST(Table, TestPointerData)
{
    CTable table(4);
    CHECK(!table.IsAggregated());

    TestData** pData = reinterpret_cast<TestData**>(table.At(1));
    CHECK(pData == NULL);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        CTable::tIndex index = 1000;
        pData = reinterpret_cast<TestData**>(table.Allocate(&index));
        CHECK(pData != NULL);
        LONGS_EQUAL(i, index);
        *pData = &m_AllData[i];
    }

    for (CTable::tIndex i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        pData = reinterpret_cast<TestData**>(table.At(i));
        CHECK(pData != NULL);
        CHECK(*pData == &m_AllData[i]);
    }

    table.Release(5);
    pData = reinterpret_cast<TestData**>(table.At(5));
    CHECK(pData == NULL);
    CTable::tIndex index;
    pData = reinterpret_cast<TestData**>(table.Allocate(&index));
    CHECK(pData != NULL);
    LONGS_EQUAL(5, index);
    table.Set(5, &m_AllData[9]);
    CHECK(*pData == &m_AllData[9]);
}
