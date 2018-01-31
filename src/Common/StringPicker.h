/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_PICKER_H__
#define __COMMON_PICKER_H__

#include <cstring>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

using std::strlen;

class CStringPicker
{
public:
    CStringPicker(
        char delimitor, const char* pString, const char* pEnd = NULL) :
        m_pCurrent(pString),
        m_pEnd(pEnd),
        m_pDelimitorStr(NULL),
        m_DelimitorStrLen(0),
        m_Delimitor(delimitor)
    {
        ASSERT(pString);
    }

    CStringPicker(
        const char* pDelimitorStr, const char* pString, const char* pEnd = NULL) :
        m_pCurrent(pString),
        m_pEnd(pEnd),
        m_pDelimitorStr(pDelimitorStr),
        m_DelimitorStrLen(strlen(pDelimitorStr)),
        m_Delimitor('\0')
    {
        ASSERT(pString);
    }

    ~CStringPicker() {}

    /** GetSubString
     * Get the sub string from current position.
     * @param pOutSubStr Output parameter of the sub string
     * @param pOutLen Output parameter of the sub string length
     * @return True if there is sub-string following the delimitor, False otherwise.
     * @note The sub string is not terminated by '\0' but indicated by the length.
     */
    bool GetSubString(const char** pOutSubStr, size_t* pOutLen);

private:
    const char* m_pCurrent;
    const char* m_pEnd;
    const char* m_pDelimitorStr;
    size_t m_DelimitorStrLen;
    const char m_Delimitor;
};

#endif