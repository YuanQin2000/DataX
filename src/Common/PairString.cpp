/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "PairString.h"
#include "StringPicker.h"
#include "Tracker/Trace.h"

CPairString::CPairString(
    char delimitor, const char* pString, size_t len /* = 0*/) :
    m_bIsPair(false),
    m_pFirst(NULL),
    m_pSecond(NULL),
    m_FirstLen(0),
    m_SecondLen(0)
{
    ASSERT(delimitor != '\0');
    ASSERT(pString);

    const char* pEnd = (len == 0) ? NULL : pString + len;
    CStringPicker stringPicker(delimitor, pString, pEnd);
    Initialize(stringPicker);
}

CPairString::CPairString(
    const char* pDelimitor, const char* pString, size_t len /* = 0*/) :
    m_bIsPair(false),
    m_pFirst(NULL),
    m_pSecond(NULL),
    m_FirstLen(0),
    m_SecondLen(0)
{
    ASSERT(pDelimitor && *pDelimitor != '\0');
    ASSERT(pString);

    const char* pEnd = (len == 0) ? NULL : pString + len;
    CStringPicker stringPicker(pDelimitor, pString, pEnd);
    Initialize(stringPicker);
}

void CPairString::Initialize(CStringPicker& picker)
{
    bool bHasNext = picker.GetSubString(&m_pFirst, &m_FirstLen);
    if (!bHasNext || m_pFirst == NULL) {
        return;
    }
    bHasNext = picker.GetSubString(&m_pSecond, &m_SecondLen);
    m_bIsPair = !bHasNext;
}
