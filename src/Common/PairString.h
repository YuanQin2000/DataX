/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_PAIR_STRING_H__
#define __COMMON_PAIR_STRING_H__

#include "Common/Typedefs.h"

class CStringPicker;
class CPairString
{
public:
    CPairString(char delimitor, const char* pString, size_t len = 0);
    CPairString(const char* pDelimitor, const char* pString, size_t len = 0);
    ~CPairString() {}

    bool IsPair() const { return m_bIsPair; }
    const char* GetFirst(size_t* pOutLen) const
    {
        *pOutLen = m_FirstLen;
        return m_pFirst;
    }
    const char* GetSecond(size_t* pOutLen) const
    {
        *pOutLen = m_SecondLen;
        return m_pSecond;
    }

private:
    void Initialize(CStringPicker& picker);

private:
    bool m_bIsPair;
    const char* m_pFirst;
    const char* m_pSecond;
    size_t m_FirstLen;
    size_t m_SecondLen;

    DISALLOW_DEFAULT_CONSTRUCTOR(CPairString);
};

#endif