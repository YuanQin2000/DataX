/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Common/DuplexList.h"
#include "TestObject.h"
#include "Defines.h"
#include <cstring>
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

using std::strcmp;

#define CHECK_LIST_CONTENT(list, iter, iterEnd, startIndex, count) do { \
    size_t curIndex = (startIndex); \
    while (iter != iterEnd) { \
        if (list.IsAggregated()) {\
            STRCMP_EQUAL( \
                m_TestData[curIndex], \
                reinterpret_cast<const char*>(list.DataAt(iter))); \
        } else { \
            STRCMP_EQUAL( \
                m_TestData[curIndex], \
                reinterpret_cast<CTestObject*>(list.DataAt(iter))->Name()); \
        } \
        CHECK(curIndex < (count) + (startIndex)); \
        ++curIndex; \
        ++iter; \
    } \
} while (0)

static CDuplexList::Iterator Find(CDuplexList& link, CTestObject* pObject)
{
    CDuplexList::Iterator iter = link.Begin();
    CDuplexList::Iterator iterEnd = link.End();
    while (iter != iterEnd) {
        if (reinterpret_cast<CTestObject*>(link.DataAt(iter)) == pObject) {
            break;
        }
        ++iter;
    }
    return iter;
}

static CDuplexList::Iterator Find(CDuplexList& link, const char* pName)
{
    CDuplexList::Iterator iter = link.Begin();
    CDuplexList::Iterator iterEnd = link.End();
    while (iter != iterEnd) {
        if (strcmp(reinterpret_cast<const char*>(link.DataAt(iter)), pName) == 0) {
            break;
        }
        ++iter;
    }
    return iter;
}

static void ReleaseResources(CDuplexList& list)
{
    CDuplexList::Iterator iter = list.Begin();
    CDuplexList::Iterator iterEnd = list.End();

    while (iter != iterEnd) {
        CTestObject* pObj = reinterpret_cast<CTestObject*>(list.DataAt(iter));
        delete pObj;
        ++iter;
    }
    list.Reset();
}

TEST_GROUP(DuplexList)
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

};

TEST(DuplexList, TestPointerInsertAtTheEnd)
{
    CDuplexList listObj;

    LONGS_EQUAL(0, listObj.Count());

    CTestObject* pFirst = reinterpret_cast<CTestObject*>(listObj.First());
    CTestObject* pLast = reinterpret_cast<CTestObject*>(listObj.Last());
    CHECK(pFirst == NULL);
    CHECK(pLast == NULL);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_TestData); ++i) {
        bool bRes = listObj.InsertBefore(listObj.End(), new CTestObject(m_TestData[i]));
        CHECK(bRes);
    }
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData), listObj.Count());
    CDuplexList::Iterator iter = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, COUNT_OF_ARRAY(m_TestData));

    pFirst = reinterpret_cast<CTestObject*>(listObj.First());
    pLast = reinterpret_cast<CTestObject*>(listObj.Last());
    CHECK(pFirst != NULL);
    CHECK(pLast != NULL);
    STRCMP_EQUAL(m_TestData[0], pFirst->Name());
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1], pLast->Name());

    listObj.PopFront();
    delete pFirst;
    pFirst = reinterpret_cast<CTestObject*>(listObj.First());
    pLast = reinterpret_cast<CTestObject*>(listObj.Last());
    CHECK(pFirst != NULL);
    CHECK(pLast != NULL);
    STRCMP_EQUAL(m_TestData[1], pFirst->Name());
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1], pLast->Name());
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 1, listObj.Count());

    ReleaseResources(listObj);
}

TEST(DuplexList, TestPointerInsert)
{
    CDuplexList listObj;

    // list: Second <-> Third <-> Fourth <-> Fifth
    for (size_t i = 1; i < 5; ++i) {
        listObj.InsertBefore(listObj.End(), new CTestObject(m_TestData[i]));
    }
    LONGS_EQUAL(4, listObj.Count());
    CDuplexList::Iterator iter = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.PositionAt(4);
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 1, 4);

    // list: First <-> Second <-> Third <-> Fourth <-> Fifth
    // Insert at the beginning
    iter = listObj.Begin();
    listObj.InsertBefore(iter, new CTestObject(m_TestData[0]));
    iter = listObj.Begin();
    iterEnd = listObj.PositionAt(5);
    LONGS_EQUAL(5, listObj.Count());
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, 5);

    // list: First <-> Second <-> Third <-> Fourth <-> Fifth <-> Sixth
    // same as pushback
    // Insert at the end
    iter = listObj.Begin();
    listObj.InsertBefore(listObj.End(), new CTestObject(m_TestData[5]));
    iterEnd = listObj.End();
    LONGS_EQUAL(6, listObj.Count());
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, 6);

    // Insert at the middle
    CDuplexList::Iterator iterMid = listObj.PositionAt(listObj.Count() / 2);
    CDuplexList::Iterator iterMidPre = listObj.PositionAt(listObj.Count() / 2 - 1);
    CDuplexList::Iterator iterMidNext = listObj.PositionAt(listObj.Count() / 2 + 1);
    listObj.InsertBefore(iterMid, new CTestObject("AAA"));
    LONGS_EQUAL(7, listObj.Count());
    listObj.InsertBefore(iterMid, new CTestObject("BBB"));
    LONGS_EQUAL(8, listObj.Count());
    listObj.InsertBefore(iterMid, new CTestObject("CCC"));
    LONGS_EQUAL(9, listObj.Count());

    ++iterMidPre;
    STRCMP_EQUAL("AAA", reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    ++iterMidPre;
    STRCMP_EQUAL("BBB", reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    ++iterMidPre;
    STRCMP_EQUAL("CCC", reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    ++iterMidPre;
    STRCMP_EQUAL(reinterpret_cast<CTestObject*>(listObj.DataAt(iterMid))->Name(),
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    ++iterMidPre;
    STRCMP_EQUAL(reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidNext))->Name(),
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());

    ReleaseResources(listObj);
}

TEST(DuplexList, TestPointerPopAndClear)
{
    CTestObject* objectArray[COUNT_OF_ARRAY(m_TestData)];
    CDuplexList listObj;

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_TestData); ++i) {
        objectArray[i] = new CTestObject(m_TestData[i]);
        listObj.PushBack(objectArray[i]);
    }

    // Find the beginnning
    CDuplexList::Iterator iterFirst = Find(listObj, objectArray[0]);
    CHECK(iterFirst != listObj.End());
    STRCMP_EQUAL(reinterpret_cast<CTestObject*>(listObj.DataAt(iterFirst))->Name(),
                 reinterpret_cast<CTestObject*>(listObj.First())->Name());

    // Erase at the beginning
    CDuplexList::Iterator iterBegin = listObj.Begin();
    listObj.PopAt(iterBegin);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 1, listObj.Count());
    iterFirst = Find(listObj, objectArray[0]);
    CHECK(iterFirst == listObj.End());

    iterBegin = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterEnd, 1, COUNT_OF_ARRAY(m_TestData) - 1);

    // Erase at the end
    CDuplexList::Iterator iterLast = Find(listObj, objectArray[COUNT_OF_ARRAY(m_TestData) - 1]);
    CHECK(iterLast != listObj.End());
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1],
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterLast))->Name());
    listObj.PopAt(iterLast);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 2, listObj.Count());
    iterLast = Find(listObj, objectArray[COUNT_OF_ARRAY(m_TestData) - 1]);
    CHECK(iterLast == listObj.End());
    iterBegin = listObj.Begin();
    iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterEnd, 1, COUNT_OF_ARRAY(m_TestData) - 2);

    // Erase at the Middle
    size_t midPos = COUNT_OF_ARRAY(m_TestData) / 2 - 1;
    CDuplexList::Iterator iterMid = Find(listObj, objectArray[midPos]);
    CHECK(iterMid != listObj.End());
    CDuplexList::Iterator iterMidPre = iterMid;
    --iterMidPre;
    CDuplexList::Iterator iterMidNext = iterMid;
    ++iterMidNext;
    CDuplexList::Iterator iterFinder = Find(listObj, objectArray[midPos - 1]);
    CHECK(iterFinder == iterMidPre);
    iterFinder = Find(listObj, objectArray[midPos + 1]);
    CHECK(iterFinder == iterMidNext);
    listObj.PopAt(iterMid);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 3, listObj.Count());
    iterFinder = Find(listObj, objectArray[midPos]);
    CHECK(iterFinder == listObj.End());
    STRCMP_EQUAL(m_TestData[midPos - 1],
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    STRCMP_EQUAL(m_TestData[midPos + 1],
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidNext))->Name());
    ++iterMidPre;
    CHECK(iterMidPre == iterMidNext);
    STRCMP_EQUAL(m_TestData[midPos + 1],
                 reinterpret_cast<CTestObject*>(listObj.DataAt(iterMidPre))->Name());
    iterBegin = listObj.Begin();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterMidPre, 1, midPos - 1);
    iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterMidPre, iterEnd, midPos + 1, COUNT_OF_ARRAY(m_TestData) - midPos - 2);

    listObj.Reset();
    LONGS_EQUAL(0, listObj.Count());

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_TestData); ++i) {
        delete objectArray[i];
    }
}

TEST(DuplexList, TestAggregatedInsertAtTheEnd)
{
    CDuplexList listObj(strlen(m_TestData[0]) + 1);

    LONGS_EQUAL(0, listObj.Count());

    const char* pFirst = reinterpret_cast<const char*>(listObj.First());
    const char* pLast = reinterpret_cast<const char*>(listObj.Last());
    CHECK(pFirst == NULL);
    CHECK(pLast == NULL);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_TestData); ++i) {
        bool bRes = listObj.InsertBefore(listObj.End(), m_TestData[i]);
        CHECK(bRes);
    }
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData), listObj.Count());
    CDuplexList::Iterator iter = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, COUNT_OF_ARRAY(m_TestData));

    pFirst = reinterpret_cast<const char*>(listObj.First());
    pLast = reinterpret_cast<const char*>(listObj.Last());
    CHECK(pFirst != NULL);
    CHECK(pLast != NULL);
    STRCMP_EQUAL(m_TestData[0], pFirst);
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1], pLast);

    listObj.PopFront();
    pFirst = reinterpret_cast<const char*>(listObj.First());
    pLast = reinterpret_cast<const char*>(listObj.Last());
    CHECK(pFirst != NULL);
    CHECK(pLast != NULL);
    STRCMP_EQUAL(m_TestData[1], pFirst);
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1], pLast);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 1, listObj.Count());
}

TEST(DuplexList, TestAggregatedInsert)
{
    CDuplexList listObj(strlen(m_TestData[0]) + 1);

    // list: Second <-> Third <-> Fourth <-> Fifth
    for (size_t i = 1; i < 5; ++i) {
        listObj.InsertBefore(listObj.End(), m_TestData[i]);
    }
    LONGS_EQUAL(4, listObj.Count());
    CDuplexList::Iterator iter = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.PositionAt(4);
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 1, 4);

    // list: First <-> Second <-> Third <-> Fourth <-> Fifth
    // Insert at the beginning
    iter = listObj.Begin();
    listObj.InsertBefore(iter, m_TestData[0]);
    iter = listObj.Begin();
    iterEnd = listObj.PositionAt(5);
    LONGS_EQUAL(5, listObj.Count());
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, 5);

    // list: First <-> Second <-> Third <-> Fourth <-> Fifth <-> Sixth
    // same as pushback
    // Insert at the end
    iter = listObj.Begin();
    listObj.InsertBefore(listObj.End(), m_TestData[5]);
    iterEnd = listObj.End();
    LONGS_EQUAL(6, listObj.Count());
    CHECK_LIST_CONTENT(listObj, iter, iterEnd, 0, 6);

    // Insert at the middle
    CDuplexList::Iterator iterMid = listObj.PositionAt(listObj.Count() / 2);
    CDuplexList::Iterator iterMidPre = listObj.PositionAt(listObj.Count() / 2 - 1);
    CDuplexList::Iterator iterMidNext = listObj.PositionAt(listObj.Count() / 2 + 1);
    listObj.InsertBefore(iterMid, "AAA0000");
    LONGS_EQUAL(7, listObj.Count());
    listObj.InsertBefore(iterMid, "BBB0000");
    LONGS_EQUAL(8, listObj.Count());
    listObj.InsertBefore(iterMid, "CCC0000");
    LONGS_EQUAL(9, listObj.Count());

    ++iterMidPre;
    STRCMP_EQUAL("AAA0000", reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    ++iterMidPre;
    STRCMP_EQUAL("BBB0000", reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    ++iterMidPre;
    STRCMP_EQUAL("CCC0000", reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    ++iterMidPre;
    STRCMP_EQUAL(reinterpret_cast<const char*>(listObj.DataAt(iterMid)),
                 reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    ++iterMidPre;
    STRCMP_EQUAL(reinterpret_cast<const char*>(listObj.DataAt(iterMidNext)),
                 reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
}

TEST(DuplexList, TestAggregatedPopAndClear)
{
    CDuplexList listObj(strlen(m_TestData[0]) + 1);

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_TestData); ++i) {
        listObj.PushBack(m_TestData[i]);
    }

    // Find the beginnning
    CDuplexList::Iterator iterFirst = Find(listObj, m_TestData[0]);
    CHECK(iterFirst != listObj.End());
    STRCMP_EQUAL(reinterpret_cast<const char*>(listObj.DataAt(iterFirst)),
                 reinterpret_cast<const char*>(listObj.First()));

    // Erase at the beginning
    CDuplexList::Iterator iterBegin = listObj.Begin();
    listObj.PopAt(iterBegin);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 1, listObj.Count());
    iterFirst = Find(listObj, m_TestData[0]);
    CHECK(iterFirst == listObj.End());

    iterBegin = listObj.Begin();
    CDuplexList::Iterator iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterEnd, 1, COUNT_OF_ARRAY(m_TestData) - 1);

    // Erase at the end
    CDuplexList::Iterator iterLast = Find(listObj, m_TestData[COUNT_OF_ARRAY(m_TestData) - 1]);
    CHECK(iterLast != listObj.End());
    STRCMP_EQUAL(m_TestData[COUNT_OF_ARRAY(m_TestData) - 1],
                 reinterpret_cast<const char*>(listObj.DataAt(iterLast)));
    listObj.PopAt(iterLast);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 2, listObj.Count());
    iterLast = Find(listObj, m_TestData[COUNT_OF_ARRAY(m_TestData) - 1]);
    CHECK(iterLast == listObj.End());
    iterBegin = listObj.Begin();
    iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterEnd, 1, COUNT_OF_ARRAY(m_TestData) - 2);

    // Erase at the Middle
    size_t midPos = COUNT_OF_ARRAY(m_TestData) / 2 - 1;
    CDuplexList::Iterator iterMid = Find(listObj, m_TestData[midPos]);
    CHECK(iterMid != listObj.End());
    CDuplexList::Iterator iterMidPre = iterMid;
    --iterMidPre;
    CDuplexList::Iterator iterMidNext = iterMid;
    ++iterMidNext;
    CDuplexList::Iterator iterFinder = Find(listObj, m_TestData[midPos - 1]);
    CHECK(iterFinder == iterMidPre);
    iterFinder = Find(listObj, m_TestData[midPos + 1]);
    CHECK(iterFinder == iterMidNext);
    listObj.PopAt(iterMid);
    LONGS_EQUAL(COUNT_OF_ARRAY(m_TestData) - 3, listObj.Count());
    iterFinder = Find(listObj, m_TestData[midPos]);
    CHECK(iterFinder == listObj.End());
    STRCMP_EQUAL(m_TestData[midPos - 1],
                 reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    STRCMP_EQUAL(m_TestData[midPos + 1],
                 reinterpret_cast<const char*>(listObj.DataAt(iterMidNext)));
    ++iterMidPre;
    CHECK(iterMidPre == iterMidNext);
    STRCMP_EQUAL(m_TestData[midPos + 1],
                 reinterpret_cast<const char*>(listObj.DataAt(iterMidPre)));
    iterBegin = listObj.Begin();
    CHECK_LIST_CONTENT(listObj, iterBegin, iterMidPre, 1, midPos - 1);
    iterEnd = listObj.End();
    CHECK_LIST_CONTENT(listObj, iterMidPre, iterEnd, midPos + 1, COUNT_OF_ARRAY(m_TestData) - midPos - 2);

    listObj.Reset();
    LONGS_EQUAL(0, listObj.Count());
}
