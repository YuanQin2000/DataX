/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <cstring>
#include <stdio.h>
#include "DataBase/KeyValueDB.h"
#include "Common/ByteData.h"
#include "Common/CharHelper.h"
#include "Tracker/Debug.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

using std::strlen;

static int IndexCompare(const char* pStr1, const char* pStr2)
{
    const char* pStrEnd1 = NSCharHelper::FindChar(':', pStr1);
    const char* pStrEnd2 = NSCharHelper::FindChar(':', pStr2);
    if (pStrEnd1 && pStrEnd2) {
        return NSCharHelper::StringCompare1(pStr1, pStrEnd1 - pStr1, pStr2, pStrEnd2 - pStr2, true);
    }
    if (pStrEnd1) {
        return 1;
    }
    if (pStrEnd2) {
        return -1;
    }
    return 0;
}

static CKeyValueDB::tRecordCompareFunc GetCompareFunc()
{
    return reinterpret_cast<CKeyValueDB::tRecordCompareFunc>(IndexCompare);
}

TEST_GROUP(KeyValueDB)
{
    const char* m_pTestKey1 = "BerkeleyDB";
    const char* m_pTestValues1[3] = {
        "A: No SQL DataBase from Berkeley UC",
        "B: Key Value Storage mode",
        "C: Embedded DataBase"
    };

    const char* m_pTestKey2 = "MySQL";
    const char* m_pTestValues2[4] = {
        "A: A pupluar open source Relation DataBase",
        "B: Support ODBC/JDBC interface",
        "C: Now owned by Oracle",
        "D: OLTP DataBase"
    };

    const char* m_pTestKey3 = "Greenplum";
    const char* m_pTestValues3[2] = {
        "A: An open source Data WareHouse",
        "B: Based on PostgreSQL"
    };

    struct KeyValueData {
        const char* pKeyString;
        const char** pValueStrings;
        size_t valuesCount;
    } m_AllKeyValues[3] = {
        { m_pTestKey1, m_pTestValues1, sizeof(m_pTestValues1) / sizeof(m_pTestValues1[0]) },
        { m_pTestKey2, m_pTestValues2, sizeof(m_pTestValues2) / sizeof(m_pTestValues2[0]) },
        { m_pTestKey3, m_pTestValues3, sizeof(m_pTestValues3) / sizeof(m_pTestValues3[0]) }
    };

    CKeyValueDB* m_pDBEngine = NULL;

void setup()
{
    SETUP_MEMORY_LEAK_CHECK;
    m_pDBEngine = CKeyValueDB::CreateInstance("./", "testKV.db", GetCompareFunc(), true);
}

void teardown()
{
    delete m_pDBEngine;
    m_pDBEngine = NULL;
}

bool SetMultiValues(KeyValueData* pData)
{
    CByteData key(const_cast<char*>(pData->pKeyString), strlen(pData->pKeyString) + 1);
    for (size_t i = 0; i < pData->valuesCount; ++i) {
        CByteData value(const_cast<char*>(
            pData->pValueStrings[i]), strlen(pData->pValueStrings[i]) + 1);
        if (!m_pDBEngine->SetValue(&key, &value)) {
            return false;
        }
    }
    return true;
}

static bool IsBelongTo(const char* pValue, KeyValueData* pData)
{
    bool bFound = false;
    size_t size = strlen(pValue) + 1;
    for (size_t i = 0; i < pData->valuesCount; ++i) {
        if (memcmp(pData->pValueStrings[i], pValue, size) == 0) {
            bFound = true;
            break;
        }
    }
    return bFound;
}

bool InsertAllData()
{
    bool bRes = true;
    for (size_t i = 0; i < sizeof(m_AllKeyValues) / sizeof(m_AllKeyValues[0]); ++i) {
        bRes = SetMultiValues(&m_AllKeyValues[i]);
        if (!bRes) {
            break;
        }
    }
    return bRes;
}

void DeleteAllData()
{
    for (size_t i = 0; i < sizeof(m_AllKeyValues) / sizeof(m_AllKeyValues[0]); ++i) {
        CByteData key(const_cast<char*>(
            m_AllKeyValues[i].pKeyString),
            strlen(m_AllKeyValues[i].pKeyString) + 1);
        m_pDBEngine->DeleteAllRecords(&key);
    }
}

static CKeyValueDB::ActionOnRecord HandleRecord(
    CKeyValueDB::Iterator* pRecord, void* pData)
{
    KeyValueData* pTestData = reinterpret_cast<KeyValueData*>(pData);
    bool bIsBelong = IsBelongTo(
        reinterpret_cast<const char*>(pRecord->Value()->GetData()), pTestData);
    CHECK(bIsBelong);
    return CKeyValueDB::ACTION_NONE;
}

};

TEST(KeyValueDB, CreateInstance)
{
    CHECK(m_pDBEngine != NULL);
}

TEST(KeyValueDB, SetGetSingleValue)
{
    bool bRes = false;
    KeyValueData* pTestData = &m_AllKeyValues[0];
    CByteData key(const_cast<char*>(pTestData->pKeyString),
                  strlen(pTestData->pKeyString) + 1);
    CByteData setValue(const_cast<char*>(pTestData->pValueStrings[0]),
                       strlen(pTestData->pValueStrings[0]) + 1);
    bRes = m_pDBEngine->SetValue(&key, &setValue);
    CHECK(bRes);

    CByteData getValue;
    bRes = m_pDBEngine->GetValue(&key, &getValue);
    CHECK(bRes);

    STRCMP_EQUAL(pTestData->pValueStrings[0], reinterpret_cast<const char*>(getValue.GetData()));

    bRes = m_pDBEngine->DeleteAllRecords(&key);
    CHECK(bRes);
}

TEST(KeyValueDB, SetGetMultiValues)
{
    KeyValueData* pTestData = &m_AllKeyValues[0];
    bool bRes = SetMultiValues(pTestData);
    CHECK(bRes);

    CByteData key(const_cast<char*>(pTestData->pKeyString), strlen(pTestData->pKeyString) + 1);
    bRes = m_pDBEngine->GetAllRecords(&key, HandleRecord, pTestData);
    CHECK(bRes);

    bRes = m_pDBEngine->DeleteAllRecords(&key);
    CHECK(bRes);
}

TEST(KeyValueDB, CursorQuery)
{
    bool bRes = InsertAllData();
    CHECK(bRes);

    ///////////////////////////////////////////////////////////////////////////
    // Query all items one by one.
    CKeyValueDB::Iterator* pIterator = m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::NORMAL);
    CHECK(pIterator != NULL);
    while (!pIterator->IsEnd()) {
        KeyValueData* pFound = NULL;
        for (size_t i = 0; i < sizeof(m_AllKeyValues) / sizeof(m_AllKeyValues[0]); ++i) {
            KeyValueData* pData = &m_AllKeyValues[i];
            if (memcmp(pData->pKeyString,
                pIterator->Key()->GetData(),
                       pIterator->Key()->GetLength()) == 0) {
                pFound = pData;
                break;
            }
        }
        CHECK(pFound != NULL);
        bool bRes = IsBelongTo(reinterpret_cast<char*>(pIterator->Value()->GetData()), pFound);
        CHECK(bRes);
        pIterator->GoNext();
    }
    CHECK(pIterator->IsEnd());
    delete pIterator;

    ///////////////////////////////////////////////////////////////////////////
    // Query all items bound to a specific key.
    CByteData key;
    KeyValueData* pTestData = &m_AllKeyValues[0];
    key.SetData(const_cast<char*>(pTestData->pKeyString), strlen(pTestData->pKeyString) + 1);
    pIterator = m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::KEY_FIXED, &key);
    CHECK(pIterator != NULL);

    size_t index = 0;
    size_t itemCount = pIterator->Count();
    LONGS_EQUAL(pTestData->valuesCount, itemCount);
    while (!pIterator->IsEnd()) {
        bool bRes = IsBelongTo(reinterpret_cast<char*>(pIterator->Value()->GetData()), pTestData);
        CHECK(bRes);
        STRCMP_EQUAL(pTestData->pKeyString, reinterpret_cast<char*>(pIterator->Key()->GetData()));
        pIterator->GoNext();
        ++index;
    }
    CHECK(pIterator->IsEnd());
    LONGS_EQUAL(itemCount, index);
    delete pIterator;

    DeleteAllData();
}

TEST(KeyValueDB, CursorErase)
{
    bool bRes = InsertAllData();
    CHECK(bRes);

    KeyValueData* pTestData = &m_AllKeyValues[0];
    CByteData key(const_cast<char*>(pTestData->pKeyString), strlen(pTestData->pKeyString) + 1);
    CKeyValueDB::Iterator* pIterator = m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::KEY_FIXED, &key);
    CHECK(pIterator != NULL);

    char* pErasedString = strdup(reinterpret_cast<char*>(pIterator->Value()->GetData()));
    size_t count = pIterator->Count();
    pIterator->EraseRecord();
    LONGS_EQUAL(count - 1, pIterator->Count());
    pIterator->GoNext();
    while (!pIterator->IsEnd()) {
        CHECK(IsBelongTo(reinterpret_cast<char*>(pIterator->Value()->GetData()), pTestData));
        CHECK(strcmp(reinterpret_cast<char*>(pIterator->Value()->GetData()), pErasedString) != 0);
        pIterator->GoNext();
    }
    delete pIterator;

    pIterator = m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::KEY_FIXED, &key);
    CHECK(pIterator != NULL);
    LONGS_EQUAL(count - 1, pIterator->Count());
    while (!pIterator->IsEnd()) {
        CHECK(IsBelongTo(reinterpret_cast<char*>(pIterator->Value()->GetData()), pTestData));
        CHECK(strcmp(reinterpret_cast<char*>(pIterator->Value()->GetData()), pErasedString) != 0);
        pIterator->GoNext();
    }
    delete pIterator;

    free(pErasedString);
    DeleteAllData();
}

TEST(KeyValueDB, CursorUpdate)
{
    bool bRes = InsertAllData();
    CHECK(bRes);

    KeyValueData* pTestData = &m_AllKeyValues[0];
    CByteData key(const_cast<char*>(
        pTestData->pKeyString), strlen(pTestData->pKeyString) + 1);
    CByteData value(const_cast<char*>(
        pTestData->pValueStrings[0]), strlen(pTestData->pValueStrings[0]) + 1);
    CKeyValueDB::Iterator* pIterator =
        m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::KEY_FIXED, &key, &value);
    CHECK(pIterator != NULL);
    CHECK(!pIterator->IsEnd());
    const char* pNewStr = "A: Replacement Value";
    CByteData newValue(const_cast<char*>(pNewStr), strlen(pNewStr) + 1);
    bRes = pIterator->UpdateValue(&newValue);
    CHECK(bRes);
    delete pIterator;

    bool bHasNonEqualValue = false;
    pIterator = m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::KEY_FIXED, &key);
    CHECK(pIterator != NULL);
    CHECK(!pIterator->IsEnd());
    while (!pIterator->IsEnd()) {
        STRCMP_EQUAL(pTestData->pKeyString, reinterpret_cast<const char*>(pIterator->Key()->GetData()));
        bool bBelong = IsBelongTo(reinterpret_cast<const char*>(pIterator->Value()->GetData()), pTestData);
        if (!bBelong) {
            bHasNonEqualValue = true;
            STRCMP_EQUAL(pNewStr, reinterpret_cast<const char*>(pIterator->Value()->GetData()));
        }
        pIterator->GoNext();
    }
    CHECK(bHasNonEqualValue);
    delete pIterator;

    DeleteAllData();
}
