/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_STRING_MAP_H__
#define __COMMON_STRING_MAP_H__

#include <map>
#include "Memory/LazyBuffer.h"

using std::map;

typedef map<const char*, const char*, tStringCompareFunc> tStringMap;

class CStringMap
{
public:
    CStringMap();
    ~CStringMap();

public:
    const char* GetValue(const char* pKey) const;
    bool SetValue(const char* pKey, const char* pValue, bool bChecked = true);
    bool Print(
        char* pBuffer, size_t len,
        const char* pSeparator,
        const char* pDelimiter,
        size_t* pOutPrintLen);
    const tStringMap& Map() const { return m_StringValues; }

    static size_t ReadKeyValue(char* pBuffer,
                               size_t len,
                               const char* pKey,
                               const char* pValues,
                               const char* pSeparator,
                               size_t separatorLen,
                               const char* pDelimiter,
                               size_t delimiterLen);

private:
    tStringMap  m_StringValues;
    CLazyBuffer m_StringBuffer;
};

#endif