/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "StringIndex.h"
#include <cstring>
#include "Tracker/Trace.h"

using std::memcmp;
using std::pair;

CStringIndex* CStringIndex::CreateInstance(
    size_t count, const char* pStrings[], bool bCaseSensitive /* = true */)
{
    ASSERT(count > 0);
    ASSERT(pStrings);

    static const size_t MAX_ARRAY_SIZE = 5;
    CStringIndex* pInstance;
    if (count >= MAX_ARRAY_SIZE) {
        pInstance = new CMapStringIndex(count, pStrings, bCaseSensitive);
    } else {
        pInstance = new CDynamicArrayStringIndex(count, pStrings, bCaseSensitive);
    }
    if (pInstance) {
        if (!pInstance->Initialized()) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

int CDynamicArrayStringIndex::GetIndexByString(
    const char* pString, size_t len /* = 0 */) const
{
    size_t i = 0;
    int index = -1;
    if (len) {
        int (*pCmpFunc)(const char*, const char*, size_t) =
            m_bCaseSensitive ? strncmp : strncasecmp;
        while (i < m_Count) {
            if (pCmpFunc(m_pStrings[i], pString, len) == 0) {
                index = static_cast<int>(i);
                break;
            }
            ++i;
        }
    } else {
        int (*pCmpFunc)(const char*, const char*) =
            m_bCaseSensitive ? strcmp : strcasecmp;
        while (i < m_Count) {
            if (pCmpFunc(m_pStrings[i], pString) == 0) {
                index = static_cast<int>(i);
                break;
            }
            ++i;
        }
    }
    return index;
}


CMapStringIndex::CMapStringIndex(
    size_t count, const char* pStrings[], bool bCaseSensitive) :
    CStringIndex(count, pStrings, bCaseSensitive),
    m_IndexMap()
{
}

int CMapStringIndex::GetIndexByString(
    const char* pString, size_t len /* = 0 */) const
{
    int res = -1;
    CLenString lenString(pString, m_bCaseSensitive, len);
    map<CLenString, int>::const_iterator iter = m_IndexMap.find(lenString);
    if (iter != m_IndexMap.end()) {
        res = iter->second;
    }
    return res;
}

bool CMapStringIndex::Initialized()
{
    for (int i = 0; i < static_cast<int>(m_Count); ++i) {
        pair<map<CLenString, int>::iterator, bool> ret =
            m_IndexMap.insert(pair<CLenString, int>(CLenString(m_pStrings[i], m_bCaseSensitive), i));
        if (!ret.second) {
            OUTPUT_WARNING_TRACE("Map insert failed at %d (%s)\n", i, m_pStrings[i]);
            return false;
        }
    }
    return true;
}

