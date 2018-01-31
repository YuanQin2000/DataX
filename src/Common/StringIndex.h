/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_STRING_INDEX_H__
#define __COMMON_STRING_INDEX_H__

#include <map>
#include <cstring>
#include "Common/Typedefs.h"
#include "LenString.h"
#include "Tracker/Trace.h"

using std::map;
using std::strlen;

class CStringIndex
{
public:
    virtual ~CStringIndex() {}
    virtual int GetIndexByString(const char* pString, size_t len = 0) const = 0;

    const char* GetStringByIndex(int index) const
    {
        ASSERT(index >= 0 && static_cast<size_t>(index) < m_Count);
        return m_pStrings[index];
    }

    size_t Count() const { return m_Count; }

    static CStringIndex* CreateInstance(
        size_t count, const char* pStrings[], bool bCaseSensitive = true);

protected:
    CStringIndex(size_t count, const char* pStrings[], bool bCaseSensitive) :
        m_Count(count),
        m_pStrings(pStrings),
        m_bCaseSensitive(bCaseSensitive) {}

private:
    virtual bool Initialized() { return true; }

protected:
    size_t m_Count;
    const char** m_pStrings;
    const bool m_bCaseSensitive;
};


class CDynamicArrayStringIndex : public CStringIndex
{
public:
    CDynamicArrayStringIndex(
        size_t count, const char* pStrings[], bool bCaseSensitive) :
        CStringIndex(count, pStrings, bCaseSensitive) {}
    ~CDynamicArrayStringIndex() {}

    int GetIndexByString(const char* pString, size_t len = 0) const;
};


class CMapStringIndex : public CStringIndex
{
public:
    CMapStringIndex(size_t count, const char* pStrings[], bool bCaseSensitive);
    ~CMapStringIndex() {}

    int GetIndexByString(const char* pString, size_t len = 0) const;

private:
    bool Initialized();

    map<CLenString, int> m_IndexMap;
};

#endif