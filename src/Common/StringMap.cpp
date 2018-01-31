/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "StringMap.h"
#include <cstring>
#include "Tracker/Trace.h"
#include "Common/CharHelper.h"

using std::pair;
using std::strlen;

CStringMap::CStringMap() :
    m_StringValues(NSCharHelper::StringCompare),
    m_StringBuffer()
{
}

CStringMap::~CStringMap()
{
}

const char* CStringMap::GetValue(const char* pKey) const
{
    ASSERT(pKey);

    tStringMap::const_iterator iter = m_StringValues.find(pKey);
    if (iter != m_StringValues.end()) {
        return iter->second;
    }
    return NULL;
}

bool CStringMap::SetValue(const char* pKey, const char* pString, bool bChecked /* = true */)
{
    tStringMap::iterator iter = m_StringValues.find(pKey);
    if (iter != m_StringValues.end()) { // Found existed item
        if (bChecked) {
            return false;
        }
        iter->second = pString;
        return true;
    }

    const char* pK = m_StringBuffer.StoreString(pKey);
    if (pK) {
        pair<tStringMap::const_iterator, bool> ret = 
            m_StringValues.insert(tStringMap::value_type(pK, pString));
        ASSERT(ret.second);
        return true;
    }
    return false;
}

bool CStringMap::Print(
    char* pBuffer,
    size_t len,
    const char* pSeparator,
    const char* pDelimiter,
    size_t* pOutPrintLen)
{
    ASSERT(len > 0);
    ASSERT(pSeparator);
    ASSERT(*pSeparator != '\0');
    ASSERT(pDelimiter);
    ASSERT(*pDelimiter != '\0');

    size_t copied = 0;
    size_t totalCopied = 0;
    bool bFinished = true;
    char* pCur = pBuffer;
    char* pEnd = pBuffer + len;
    tStringMap::const_iterator iter = m_StringValues.begin();
    tStringMap::const_iterator iterEnd = m_StringValues.end();

    while (iter != iterEnd) {
        // Copy the Key
        bFinished = NSCharHelper::CopyNChars(
                    pCur, pEnd - pCur, iter->first, &copied);
        pCur += copied;
        totalCopied += copied;
        if (pCur == pEnd) {
            bFinished = false;
        }
        if (!bFinished) {
            break;
        }

        // Copy Separator String
        bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, pSeparator, &copied);
        pCur += copied;
        totalCopied += copied;
        if (pCur == pEnd) {
            bFinished = false;
        }
        if (!bFinished) {
            break;
        }

        // Copy the Value
        bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, iter->second, &copied);
        pCur += copied;
        totalCopied += copied;
        if (pCur == pEnd) {
            bFinished = false;
        }
        if (!bFinished) {
            break;
        }

        // Append the delimitor string.
        bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, pDelimiter, &copied);
        pCur += copied;
        totalCopied += copied;
        if (pCur == pEnd) {
            bFinished = false;
        }
        if (!bFinished) {
            break;
        }

        ++iter;
    }
    *pOutPrintLen = totalCopied;
    return bFinished;
}
