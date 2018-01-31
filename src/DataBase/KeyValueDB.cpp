/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "KeyValueDB.h"
#include <map>
#include <utility>
#include "Common/ByteData.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::map;
using std::pair;

CCriticalSection CKeyValueDB::s_CS;
CKeyValueDB::tEnvMap CKeyValueDB::s_EnvMap(NSCharHelper::StringCompare);
const char* CKeyValueDB::s_pDBErrorMsgPrefix = "KeyValue DB";

CKeyValueDB::CKeyValueDB(char* pPath, tRecordCompareFunc pCompareFunc) :
    m_pDBHandle(NULL),
    m_pWorkDir(pPath),
    m_pDBEnv(NULL),
    m_pClientCompareFunc(pCompareFunc)
{
}

CKeyValueDB::~CKeyValueDB()
{
    if (m_pDBHandle) {
        if (m_pDBHandle->api_internal == this) {
            m_pDBHandle->api_internal = NULL;
        }
        m_pDBHandle->close(m_pDBHandle, 0);
        m_pDBHandle = NULL;
    }
    if (m_pWorkDir) {
        if (m_pDBEnv) {
            ReleaseEnvironment(m_pWorkDir);
            m_pDBEnv = NULL;
        }
        free(m_pWorkDir);
    }
}

CKeyValueDB* CKeyValueDB::CreateInstance(
    const char* pWorkDir,
    const char* pDBName,
    tRecordCompareFunc pCompareFunc,
    bool bMultipleValue)
{
    ASSERT(pWorkDir);
    ASSERT(pDBName);

    size_t dbWorkDirLen = strlen(pWorkDir) + 1;
    size_t dbNameLen = strlen(pDBName) + 1;

    ASSERT(dbNameLen > 0);
    ASSERT(dbWorkDirLen > 0);

    char* pPath = reinterpret_cast<char*>(malloc(dbWorkDirLen + dbNameLen));
    if (pPath == NULL) {
        OUTPUT_WARNING_TRACE("malloc failed.\n");
        return NULL;
    }
    char* pFile = pPath + dbWorkDirLen;
    memcpy(pPath, pWorkDir, dbWorkDirLen);
    memcpy(pFile, pDBName, dbNameLen);

    CKeyValueDB* pInstance = new CKeyValueDB(pPath, pCompareFunc);
    if (pInstance == NULL) {
        free(pPath);
        return NULL;
    }

    if ((pInstance->m_pDBEnv = GetEnvironment(pPath)) == NULL) {
        delete pInstance;
        return NULL;
    }

    bool bRes = false;
    int err = db_create(&pInstance->m_pDBHandle, pInstance->m_pDBEnv, 0);
    if (err == 0) {
        ASSERT(pInstance->m_pDBHandle);
        //m_pDBHandle->set_errcall(m_pDBHandle, ErrorMsgHandler);
        //m_pDBHandle->set_errpfx(m_pDBHandle, s_pDBErrorMsgPrefix);
        if (bMultipleValue) {
            pInstance->m_pDBHandle->set_flags(pInstance->m_pDBHandle, DB_DUPSORT);
            if (pCompareFunc) {
                pInstance->m_pDBHandle->set_dup_compare(pInstance->m_pDBHandle, RecordCompare);
                pInstance->m_pDBHandle->api_internal = pInstance;
            }
        }
        err = pInstance->m_pDBHandle->open(
            pInstance->m_pDBHandle, NULL, pFile, NULL, DB_BTREE, DB_CREATE, 0);
        if (err == 0) {
            bRes = true;
        } else {
            OUTPUT_ERROR_TRACE("open (DB: %s): %s\n", pFile, db_strerror(err));
        }
    } else {
        OUTPUT_ERROR_TRACE("db_create: %s\n", db_strerror(err));
    }

    if (!bRes) {
        delete pInstance;
        pInstance = NULL;
    }
    return pInstance;
}

bool CKeyValueDB::SetValue(const CByteData* pKey, const CByteData* pValue)
{
    DBT keyDBT;
    DBT valueDBT;
    memset(&keyDBT, 0, sizeof(keyDBT));
    memset(&valueDBT, 0, sizeof(valueDBT));
    keyDBT.data = pKey->GetData();
    keyDBT.size = pKey->GetLength();
    valueDBT.data = pValue->GetData();
    valueDBT.size = pValue->GetLength();
    bool bRes = true;
    int res = m_pDBHandle->put(m_pDBHandle, NULL, &keyDBT, &valueDBT, 0);
    if (res != 0 && res != DB_KEYEXIST) {
        bRes = false;
    }
    return bRes;
}

CByteData* CKeyValueDB::GetValue(const CByteData* pKey, CByteData* pOutValue)
{
    DBT keyDBT;
    DBT valueDBT;
    memset(&keyDBT, 0, sizeof(keyDBT));
    memset(&valueDBT, 0, sizeof(valueDBT));
    keyDBT.data = pKey->GetData();
    keyDBT.size = pKey->GetLength();
    valueDBT.flags = DB_DBT_MALLOC;
    int res = m_pDBHandle->get(m_pDBHandle, NULL, &keyDBT, &valueDBT, 0);
    if (res != 0) {
        OUTPUT_WARNING_TRACE("DB: get: %s\n", db_strerror(res));
        if (valueDBT.data) {
            free(valueDBT.data);
            valueDBT.data = NULL;
            valueDBT.size = 0;
        }
    }
    pOutValue->SetData(valueDBT.data, valueDBT.size, free);
    return pOutValue;
}

bool CKeyValueDB::GetAllRecords(
    const CByteData* pKey, tRecordHandle handler, void* pData)
{
    Iterator::Type type = Iterator::NORMAL;
    if (pKey) {
        ASSERT(!pKey->IsNull());
        type = Iterator::KEY_FIXED;
    }
    Iterator* pIterator = CreateIterator(type, pKey);
    if (pIterator == NULL) {
        return false;
    }

    while (!pIterator->IsEnd()) {
        handler(pIterator, pData);
        if (!pIterator->GoNext()) {
            break;
        }
    }

    bool bRes = true;
    if (!pIterator->IsEnd()) {
        OUTPUT_WARNING_TRACE("Not get all the DB records\n");
        bRes = false;
    }
    delete pIterator;
    return bRes;
}

bool CKeyValueDB::DeleteAllRecords(const CByteData* pKey)
{
    DBT keyDBT;
    memset(&keyDBT, 0, sizeof(keyDBT));
    keyDBT.data = pKey->GetData();
    keyDBT.size = pKey->GetLength();
    int res = m_pDBHandle->del(m_pDBHandle, NULL, &keyDBT, 0);
    return (res == 0 || res == DB_NOTFOUND || res == DB_KEYEMPTY);
}

CKeyValueDB::Iterator* CKeyValueDB::CreateIterator(
    Iterator::Type type /* = Iterator::NORMAL */,
    const CByteData* pPosKey /* = NULL */,
    const CByteData* pPosValue /* = NULL */)
{
    DBC* pCursor = NULL;
    int res = m_pDBHandle->cursor(m_pDBHandle, NULL, &pCursor, 0);
    if (res != 0) {
        OUTPUT_ERROR_TRACE("DB cursor: %s\n", db_strerror(res));
        if (pCursor) {
            pCursor->close(pCursor);
            pCursor = NULL;
        }
    }

    Iterator* pInstance = NULL;
    if (pCursor) {
        pInstance = Iterator::CreateInstance(pCursor, type, pPosKey, pPosValue);
    }
    return pInstance;
}

DB_ENV* CKeyValueDB::GetEnvironment(const char* pWorkDir)
{
    DB_ENV* pEnv = NULL;
    CSectionLock lock(s_CS);
    tEnvMap::iterator iter = s_EnvMap.find(pWorkDir);
    if (iter != s_EnvMap.end()) {
        EnvironmentData* pData = iter->second;
        ASSERT(pData);
        ASSERT(pData->pEnvironment);
        ASSERT(pData->RefCount >= 1);
        ++pData->RefCount;
        pEnv = pData->pEnvironment;
    } else {
        EnvironmentData* pData = new EnvironmentData();
        pEnv = CreateEnvironment(pWorkDir);
        if (pData && pEnv) {
            pData->pEnvironment = pEnv;
            pData->RefCount = 1;
            s_EnvMap.insert(tEnvMap::value_type(pWorkDir, pData));
        } else {
            delete pData;
            if (pEnv) {
                DeleteDBEnvironment(pEnv);
            }
            pEnv = NULL;
        }
    }
    return pEnv;
}

void CKeyValueDB::ReleaseEnvironment(const char* pWorkDir)
{
    CSectionLock lock(s_CS);
    tEnvMap::iterator iter = s_EnvMap.find(pWorkDir);
    if (iter != s_EnvMap.end()) {
        EnvironmentData* pData = iter->second;
        ASSERT(pData);
        ASSERT(pData->pEnvironment);
        ASSERT(pData->RefCount >= 1);
        if (--pData->RefCount == 0) {
            DeleteDBEnvironment(pData->pEnvironment);
            delete pData;
        }
        s_EnvMap.erase(iter);
    }
}

DB_ENV* CKeyValueDB::CreateEnvironment(const char* pWorkDir)
{
    DB_ENV* pEnvInstance = NULL;
    int err = db_env_create(&pEnvInstance, 0);
    if (err != 0) {
        OUTPUT_ERROR_TRACE("db_env_create: %s\n", db_strerror(err));
        return NULL;
    }
    uint32_t envFlags = DB_CREATE | DB_INIT_MPOOL;
    err = pEnvInstance->open(pEnvInstance, pWorkDir, envFlags, 0);
    if (err != 0) {
        OUTPUT_ERROR_TRACE("env open: %s\n", db_strerror(err));
        pEnvInstance->close(pEnvInstance, 0);
        pEnvInstance = NULL;
    }
    return pEnvInstance;
}

void CKeyValueDB::DeleteDBEnvironment(DB_ENV* pEnv)
{
    if (pEnv) {
        pEnv->close(pEnv, 0);
    }
}

int CKeyValueDB::RecordCompare(
    DB* pDBHandler, const DBT* pRecord1, const DBT* pRecord2, size_t* locp)
{
    CKeyValueDB* pThis = reinterpret_cast<CKeyValueDB*>(pDBHandler->api_internal);  // code hack
    void* pData1 = pRecord1->data;
    void* pData2 = pRecord2->data;
    if (pData1 && pData2) {
        return pThis->m_pClientCompareFunc(pData1, pData2);
    }
    if (pData1) {
        return 1;
    }
    if (pData2) {
        return -1;
    }
    return 0;
}

void CKeyValueDB::ErrorMsgHandler(
    const DB_ENV* pDBEnv, const char* pErrMsgPrefix, const char* pErrMsg)
{
    OUTPUT_ERROR_TRACE("DB Error: %s %s\n", pErrMsgPrefix, pErrMsg);
}


///////////////////////////////////////////////////////////////////////////////
//
// Iterator Implementation
//
///////////////////////////////////////////////////////////////////////////////
CKeyValueDB::Iterator* CKeyValueDB::Iterator::CreateInstance(
    DBC* pCursor,
    Type type /* = NORMAL */,
    const CByteData* pPosKey /* = NULL */,
    const CByteData* pPosValue /* = NULL */)
{
    Iterator* pInstance =
        new CKeyValueDB::Iterator(pCursor, type, pPosKey, pPosValue);
    if (pInstance) {
        bool bRes = false;
        if (pPosKey) {
            pInstance->m_Key.SetData(pPosKey->GetData(), pPosKey->GetLength());
            if (pPosValue) {
                pInstance->m_Value.SetData(pPosValue->GetData(), pPosValue->GetLength());
                bRes = pInstance->GetData(
                    const_cast<CByteData*>(pPosKey), const_cast<CByteData*>(pPosValue), DB_GET_BOTH);
            } else {
                bRes = pInstance->GetData(
                    const_cast<CByteData*>(pPosKey), &pInstance->m_Value, DB_SET);
            }
        } else {
            bRes = pInstance->GetData(&pInstance->m_Key, &pInstance->m_Value, DB_FIRST);
        }
        if (!bRes) {
            delete pInstance;
            pInstance = NULL;
        }
    } else {
        pCursor->close(pCursor);
    }
    return pInstance;
}

CKeyValueDB::Iterator::~Iterator()
{
    if (m_pCursor) {
        m_pCursor->close(m_pCursor);
    }
}

size_t CKeyValueDB::Iterator::Count() const
{
    db_recno_t counts = 0;
    int res = m_pCursor->count(m_pCursor, &counts, 0);
    if (res != 0) {
        OUTPUT_ERROR_TRACE("DBC Count: %s\n", db_strerror((res)));
    }
    return counts;
}

bool CKeyValueDB::Iterator::GoNext()
{
    uint32_t cursorGetFlags = 0;
    switch (m_Type) {
    case Iterator::NORMAL:
        cursorGetFlags = DB_NEXT;
        break;
    case Iterator::KEY_FIXED:
        cursorGetFlags = DB_NEXT_DUP;
        break;
    case Iterator::KEY_INDEXED:
        cursorGetFlags = DB_NEXT_NODUP;
        break;
    default:
        ASSERT(false);
    }

    CByteData* pKey = NULL;
    if (!m_pPosKey) {
        pKey = &m_Key;
        pKey->SetData(NULL, 0);
    }
    m_Value.SetData(NULL, 0);
    return GetData(pKey, &m_Value, cursorGetFlags);
}

bool CKeyValueDB::Iterator::UpdateValue(const CByteData* pValue)
{
    DBT keyDBT;
    DBT valueDBT;
    memset(&valueDBT, 0, sizeof(valueDBT));
    memset(&keyDBT, 0, sizeof(keyDBT));
    keyDBT.data = m_Key.GetData();
    keyDBT.size = m_Key.GetLength();
    valueDBT.data = pValue->GetData();
    valueDBT.size = pValue->GetLength();
    return m_pCursor->put(m_pCursor, &keyDBT, &valueDBT, DB_CURRENT) == 0;
}

bool CKeyValueDB::Iterator::GetData(
    CByteData* pInOutKey, CByteData* pInOutValue, uint32_t flags)
{
    DBT keyDBT;
    DBT valueDBT;
    memset(&valueDBT, 0, sizeof(valueDBT));
    memset(&keyDBT, 0, sizeof(keyDBT));

    bool bOutKey = false;
    bool bOutValue = false;
    if (pInOutKey) {
        if (pInOutKey->IsNull()) {
            // Key as a output.
            bOutKey = true;
            keyDBT.flags = DB_DBT_MALLOC;
        } else {
            keyDBT.data = pInOutKey->GetData();
            keyDBT.size = pInOutKey->GetLength();
        }
    }
    if (pInOutValue) {
        if (pInOutValue->IsNull()) {
            // Value as a output.
            bOutValue = true;
            valueDBT.flags = DB_DBT_MALLOC;
        } else {
            valueDBT.data = pInOutValue->GetData();
            valueDBT.size = pInOutValue->GetLength();
        }
    }

    bool bRes = false;
    int res = m_pCursor->get(m_pCursor, &keyDBT, &valueDBT, flags);
    if (res == 0) {
        if (bOutKey && keyDBT.data && keyDBT.size > 0) {
            pInOutKey->SetData(keyDBT.data, keyDBT.size, free);
        }
        if (bOutValue && pInOutValue && valueDBT.data && valueDBT.size > 0) {
            pInOutValue->SetData(valueDBT.data, valueDBT.size, free);
        }
        bRes = true;
    } else {
        if (res == DB_NOTFOUND) {
            m_bReachEnd = true;
            bRes = true;
        } else {
            OUTPUT_WARNING_TRACE("DBC get: %s\n", db_strerror(res));
        }
    }

    return bRes;
}
