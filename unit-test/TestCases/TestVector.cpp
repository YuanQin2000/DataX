/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Vector.h"
#include "Common/Macros.h"
#include "Tracker/Debug.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(Vector)
{
    struct TestData {
        uint32_t Number;
        const char* pName;
    } m_AllData[10] = {
        { 1, "Item1" },
        { 2, "Item2" },
        { 3, "Item3" },
        { 4, "Item4" },
        { 5, "Item5" },
        { 6, "Item6" },
        { 7, "Item7" },
        { 8, "Item8" },
        { 9, "Item9" },
        { 0, "Item0" },
    };

void CheckResult(const char* idStr, TestData* pResult, size_t index, bool bAggregated)
{
    CHECK(pResult != NULL);
    if (bAggregated) {
        CHECK(pResult != &m_AllData[index]);
        int compareValue = memcmp(pResult, &m_AllData[index], sizeof(TestData));
        LONGS_EQUAL_TEXT(0, compareValue, idStr);
    } else {
        CHECK(pResult == &m_AllData[index]);
    }
}

void DoTest(CVector& vector)
{
    TestData* pResult = NULL;
    size_t count = 0;
    LONGS_EQUAL(count, vector.Count());
    LONGS_EQUAL(0, vector.Capacity());
    for (size_t i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        vector.PushBack(&m_AllData[i]);
        ++count;
        LONGS_EQUAL(count, vector.Count());
    }
    CHECK(vector.Capacity() >= COUNT_OF_ARRAY(m_AllData));

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_AllData); ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("0", pResult, i, vector.IsAggregated());
    }

    size_t erasePos = 3;
    vector.Erase(3);
    --count;
    LONGS_EQUAL(count, vector.Count());
    for (size_t i = 0; i < erasePos; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("1", pResult, i, vector.IsAggregated());
    }
    for (size_t i = 3; i < count; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("2", pResult, i + 1, vector.IsAggregated());
    }

    vector.Insert(erasePos, &m_AllData[erasePos]);
    ++count;
    LONGS_EQUAL(count, vector.Count());
    for (size_t i = 0; i < count; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("3", pResult, i, vector.IsAggregated());
    }

    vector.PopFront();
    --count;
    LONGS_EQUAL(count, vector.Count());
    pResult = reinterpret_cast<TestData*>(vector.At(0));
    CheckResult("4", pResult, 1, vector.IsAggregated());

    for (size_t i = 0; i < count; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("5", pResult, i + 1, vector.IsAggregated());
    }

    vector.PushFront(&m_AllData[0]);
    ++count;
    LONGS_EQUAL(count, vector.Count());
    for (size_t i = 0; i < count; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("6", pResult, i, vector.IsAggregated());
    }

    vector.PopBack();
    --count;
    LONGS_EQUAL(count, vector.Count());
    for (size_t i = 0; i < count; ++i) {
        pResult = reinterpret_cast<TestData*>(vector.At(i));
        CheckResult("7", pResult, i, vector.IsAggregated());
    }

    vector.Set(0, &m_AllData[1]);
    pResult = reinterpret_cast<TestData*>(vector.At(0));
    CheckResult("8", pResult, 1, vector.IsAggregated());

    vector.Set(count - 1, &m_AllData[1]);
    pResult = reinterpret_cast<TestData*>(vector.At(count - 1));
    CheckResult("9", pResult, 1, vector.IsAggregated());
    LONGS_EQUAL(count, vector.Count());

    vector.Clear();
    LONGS_EQUAL(0, vector.Count());
}

void setup()
{
}

void teardown()
{
}

};

TEST(Vector, TestAggregatedData)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CVector vector(sizeof(TestData), COUNT_OF_ARRAY(m_AllData) / 4);
    CHECK(vector.IsAggregated());
    DoTest(vector);
}

TEST(Vector, TestPointerData)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CVector vector(COUNT_OF_ARRAY(m_AllData) / 4);
    CHECK(!vector.IsAggregated());
    DoTest(vector);
}
