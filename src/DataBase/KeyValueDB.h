/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATABASE_KEYVALUE_DB_H__
#define __DATABASE_KEYVALUE_DB_H__

#include <map>
#include "Common/ByteData.h"
#include "Thread/Lock.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"
// Berkeley DB engine, notice it's AGPL.
#include "berkeleydb/db.h"

using std::map;

class CKeyValueDB
{
public:
    typedef int (*tRecordCompareFunc)(void*, void*);

    enum ActionOnRecord {
        ACTION_NONE,
        ACTION_UPDATED,
        ACTION_DELETED
    };

    class Iterator;
    typedef ActionOnRecord (*tRecordHandle)(Iterator* pRecord, void* pData);

    ~CKeyValueDB();

    bool SetValue(const CByteData* pKey, const CByteData* pValue);
    CByteData* GetValue(const CByteData* pKey, CByteData* pOutValue);
    void Sync() { m_pDBHandle->sync(m_pDBHandle, 0); }

    /**
     * @brief Get all records (of the specified key if key is not NULL)
     * @param pKey specify the key.
     * @param handler The function to handle one record.
     * @param pData The (context) data for the handler
     * @return True if get all records otherwise false.
     */
    bool GetAllRecords(const CByteData* pKey, tRecordHandle handler, void* pData);

    /**
     * @brief Delete all the value items of the key.
     * @param pKey specify the key.
     * @return return true if success and false if failed.
     */
    bool DeleteAllRecords(const CByteData* pKey);

    class Iterator
    {
    public:
        enum Type {
            // DB Cursor will always move 1 steps until reach end
            NORMAL,
            // DB Cursor will always move 1 steps untils the key is changed.
            KEY_FIXED,
            // DB Cursor will always move to the first value of next different key until reach end.
            KEY_INDEXED
        };

        ~Iterator();

        /**
         * @brief Return the count of current key item.
         * @note The value is the same if the cursor point to the same key (but different value)
         */
        size_t Count() const;

        bool GoNext();
        CByteData* Key()   { return &m_Key; }
        CByteData* Value() { return &m_Value; }
        bool IsEnd() const { return m_bReachEnd; }
        bool EraseRecord() { return m_pCursor->del(m_pCursor, 0) == 0; }
        bool UpdateValue(const CByteData* pValue);

    private:
        Iterator(DBC* pCursor,
                  Type type,
                  const CByteData* pPosKey = NULL,
                  const CByteData* pPosValue = NULL) :
            m_Type(type),
            m_bReachEnd(false),
            m_pCursor(pCursor),
            m_Key(),
            m_Value(),
            m_pPosKey(pPosKey),
            m_pPosValue(pPosValue) {}

        static Iterator* CreateInstance(
            DBC* pCursor,
            Type type = NORMAL,
            const CByteData* pPosKey = NULL,
            const CByteData* pPosValue = NULL);

        bool GetData(CByteData* pInOutKey, CByteData* pInOutValue, uint32_t flags);

    private:
        uint8_t m_Type;
        bool m_bReachEnd;
        DBC* m_pCursor; // Owned
        CByteData m_Key;
        CByteData m_Value;
        const CByteData* m_pPosKey;   // Not Owned
        const CByteData* m_pPosValue; // Not Owned

        friend class CKeyValueDB;

        DISALLOW_COPY_CONSTRUCTOR(Iterator);
        DISALLOW_ASSIGN_OPERATOR(Iterator);
    };

    Iterator* CreateIterator(
        Iterator::Type type = Iterator::NORMAL,
        const CByteData* pPosKey = NULL,
        const CByteData* pPosValue = NULL);

    static CKeyValueDB* CreateInstance(
        const char* pWorkDir,
        const char* pDBName,
        tRecordCompareFunc pCompareFunc,
        bool bMultipleValue);

private:
    CKeyValueDB(char* pWorkDir, tRecordCompareFunc pCompareFunc);

private:
    struct EnvironmentData {
        DB_ENV* pEnvironment;
        size_t RefCount;

        EnvironmentData() : pEnvironment(NULL), RefCount(0) {}
    };

    typedef map<const char*, EnvironmentData*, tStringCompareFunc> tEnvMap;

    static int RecordCompare(
        DB* pDBHandler, const DBT* pRecord1, const DBT* pRecord2, size_t* locp);
    static void ErrorMsgHandler(
        const DB_ENV* pDBEnv, const char* pErrMsgPrefix, const char* pErrMsg);

    static DB_ENV* GetEnvironment(const char* pWorkDir);
    static void ReleaseEnvironment(const char* pWorkDir);
    static DB_ENV* CreateEnvironment(const char* pWorkDir);
    static void DeleteDBEnvironment(DB_ENV* pEnv);

private:
    DB* m_pDBHandle;    // Owned
    char* m_pWorkDir;   // Owned
    DB_ENV* m_pDBEnv;   // Not owned
    tRecordCompareFunc m_pClientCompareFunc;

    static CCriticalSection s_CS;
    static tEnvMap s_EnvMap;

    static const char s_DBFileExtName[];
    static const char* s_pDBErrorMsgPrefix;

    DISALLOW_DEFAULT_CONSTRUCTOR(CKeyValueDB);
    DISALLOW_COPY_CONSTRUCTOR(CKeyValueDB);
    DISALLOW_ASSIGN_OPERATOR(CKeyValueDB);
};

#endif
