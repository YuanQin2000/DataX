/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/ForwardList.h"
#include "TestObject.h"
#include "Defines.h"
#include "Tracker/Debug.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(ForwardList)
{
    const char* m_TestData[10] = {
        "First00",
        "Second0",
        "Third00",
        "Fourth0",
        "Fifth00",
        "Sixth00",
        "Seventh",
        "Eighth0",
        "Ninth00",
        "Tenth00",
    };

void setup()
{
}

void teardown()
{
}

void ReleaseListItems(CForwardList& fwList)
{
    CForwardList::Iterator iter = fwList.Begin();
    CForwardList::Iterator iterEnd = fwList.End();
    while (iter != iterEnd) {
        CTestObject* pObject = reinterpret_cast<CTestObject*>(fwList.DataAt(iter));
        CHECK(pObject != NULL);
        ++iter;
        delete pObject;
    }
}

};

TEST(ForwardList, TestAggregatedData)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CForwardList fwList1(strlen(m_TestData[0]) + 1);
    CForwardList fwList2(strlen(m_TestData[0]) + 1);

    CHECK(fwList1.IsAggregated());
    CHECK(fwList2.IsAggregated());

    LONGS_EQUAL(0, fwList1.Count());
    LONGS_EQUAL(0, fwList2.Count());

    size_t list1Count = 0;
    for (size_t i = 0; i < sizeof(m_TestData) / sizeof(m_TestData[0]); ++i) {
        if (i == 6) {
            list1Count = i;
            break;
        }
        fwList1.PushBack(const_cast<char*>(m_TestData[i]));
    }
    LONGS_EQUAL(list1Count, fwList1.Count());

    CForwardList::Iterator iter = fwList1.Begin();
    CForwardList::Iterator iterEnd = fwList1.End();
    size_t idx = 0;
    while (iter != iterEnd) {
        const char* pValue = reinterpret_cast<const char*>(fwList1.DataAt(iter));
        STRCMP_EQUAL(m_TestData[idx], pValue);
        ++iter;
        ++idx;
    }
    LONGS_EQUAL(list1Count, idx);

    // Push back list2
    while (idx < sizeof(m_TestData) / sizeof(m_TestData[0])) {
        fwList2.PushBack(const_cast<char*>(m_TestData[idx]));
        ++idx;
    }
    LONGS_EQUAL(idx - list1Count, fwList2.Count());

    fwList1.PushBack(fwList2);
    LONGS_EQUAL(idx, fwList1.Count());
    LONGS_EQUAL(0, fwList2.Count());

    iter = fwList1.Begin();
    iterEnd = fwList1.End();
    idx = 0;
    while (iter != iterEnd) {
        const char* pValue = reinterpret_cast<const char*>(fwList1.DataAt(iter));
        STRCMP_EQUAL(m_TestData[idx], pValue);
        ++idx;
        ++iter;
    }
    LONGS_EQUAL(idx, fwList1.Count());
}

TEST(ForwardList, TestPointerData)
{
    CForwardList fwList1;
    CForwardList fwList2;

    CHECK(!fwList1.IsAggregated());
    CHECK(!fwList2.IsAggregated());

    LONGS_EQUAL(0, fwList1.Count());
    LONGS_EQUAL(0, fwList2.Count());

    size_t list1Count = 0;
    for (size_t i = 0; i < sizeof(m_TestData) / sizeof(m_TestData[0]); ++i) {
        if (i == 6) {
            list1Count = i;
            break;
        }
        fwList1.PushBack(new CTestObject(m_TestData[i]));
    }
    LONGS_EQUAL(list1Count, fwList1.Count());

    CForwardList::Iterator iter = fwList1.Begin();
    CForwardList::Iterator iterEnd = fwList1.End();
    size_t idx = 0;
    while (iter != iterEnd) {
        const char* pValue = reinterpret_cast<CTestObject*>(fwList1.DataAt(iter))->Name();
        STRCMP_EQUAL(m_TestData[idx], pValue);
        ++iter;
        ++idx;
    }
    LONGS_EQUAL(list1Count, idx);

    // Push back list2
    while (idx < sizeof(m_TestData) / sizeof(m_TestData[0])) {
        fwList2.PushBack(new CTestObject(m_TestData[idx]));
        ++idx;
    }
    LONGS_EQUAL(idx - list1Count, fwList2.Count());

    fwList1.PushBack(fwList2);
    LONGS_EQUAL(idx, fwList1.Count());
    LONGS_EQUAL(0, fwList2.Count());

    iter = fwList1.Begin();
    iterEnd = fwList1.End();
    idx = 0;
    while (iter != iterEnd) {
        CTestObject* pObject = reinterpret_cast<CTestObject*>(fwList1.DataAt(iter));
        CHECK(pObject != NULL);
        const char* pValue = pObject->Name();
        STRCMP_EQUAL(m_TestData[idx], pValue);
        ++idx;
        ++iter;
    }
    LONGS_EQUAL(idx, fwList1.Count());

    ReleaseListItems(fwList1);
}
