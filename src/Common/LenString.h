/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CHAR_STRING_H__
#define __CHAR_STRING_H__

#include <cstring>
#include "Common/Typedefs.h"
#include "Common/CharHelper.h"

// Char string without the null terminator ended but specific the length
class CLenString
{
public:
    CLenString(const char* pValue, bool bCaseSensitive, size_t len = 0) :
        m_pString(pValue),
        m_bCaseSensitive(bCaseSensitive),  
        m_Length(len) {}
    CLenString() :
        m_pString(""),
        m_bCaseSensitive(false),
        m_Length(0) {}

    bool operator==(const CLenString& rhs)
    {
        return (this == &rhs ||
            NSCharHelper::StringCompare1(
                m_pString,
                m_Length,
                rhs.m_pString,
                rhs.m_Length,
                m_bCaseSensitive) == 0);
    }

    bool operator<(const CLenString& rhs) const
    {
        return NSCharHelper::StringCompare1(
            m_pString, m_Length, rhs.m_pString, rhs.m_Length, m_bCaseSensitive) < 0;
    }

    inline CLenString& operator=(const CLenString& rhs)
    {
        m_pString = rhs.m_pString;
        m_Length = rhs.m_Length;
        return *this;
    }

    const char* Value() const { return m_pString; }
    size_t Length()     const { return m_Length;  }

private:
    const char* m_pString;
    bool m_bCaseSensitive;
    size_t m_Length;
};

#endif