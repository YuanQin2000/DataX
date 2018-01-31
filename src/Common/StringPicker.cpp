/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "StringPicker.h"
#include <cstring>
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::strlen;
using std::strchr;

bool CStringPicker::GetSubString(const char** pOutSubStr, size_t* pOutLen)
{
    ASSERT(pOutSubStr);
    ASSERT(pOutLen);

    bool bRes = false;
    *pOutSubStr = NULL;
    *pOutLen = 0;
    do {
        if (m_pCurrent == NULL) {
            break;
        }
        m_pCurrent = NSCharHelper::TrimLeftSpace(m_pCurrent, m_pEnd);
        if (m_pCurrent == NULL) {
            break;
        }

        const char* pBegin = m_pCurrent;
        const char* pEnd = m_pDelimitorStr ?
            NSCharHelper::FindSubStr(m_pDelimitorStr, m_DelimitorStrLen, pBegin, m_pEnd, true) :
            NSCharHelper::FindChar(m_Delimitor, pBegin, m_pEnd, true);
        if (pEnd == m_pEnd || *pEnd == '\0') {
            // The delimitor is the last character.
            m_pCurrent = NULL;
        } else {
            m_pCurrent = pEnd + 1;
        }

        pEnd = NSCharHelper::TrimRightSpace(pBegin, pEnd);
        if (pEnd) {
            *pOutSubStr = pBegin;
            *pOutLen = pEnd - pBegin;
            bRes = m_pCurrent != NULL;
            break;
        }
    } while (true);
    return bRes;
}
