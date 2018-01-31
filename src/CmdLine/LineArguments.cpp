/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "LineArguments.h"
#include <cctype>
#include "Common/CharHelper.h"
#include "Common/StringPicker.h"

using std::isspace;

///////////////////////////////////////////////////////////////////////////////
//
// CLineArguments Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CLineArguments::CLineArguments(char* pRawData) :
    m_pRawData(pRawData),
    m_pFileName(NULL),
    m_ArgData(sizeof(ArgumentData), 8),
    m_ArgDataBackup(sizeof(ArgumentData), 8)
{
}

CLineArguments::~CLineArguments()
{
    if (m_pRawData) {
        free(m_pRawData);
    }
}

CLineArguments* CLineArguments::CreateInstance(
    char* pTextLine,
    bool bTransferOwnerhip /* = false */)
{
    CStringPicker strPicker(' ', pTextLine);
    char* pOwnedData = bTransferOwnerhip ? pTextLine : NULL;
    CLineArguments* pInstance = new CLineArguments(pOwnedData);
    if (pInstance == NULL) {
        OUTPUT_ERROR_TRACE("Allocate memory failed\n");
        return NULL;
    }

    char* pSubStr = NULL;
    size_t subLen = 0;
    bool bHasNext = false;
    do {
        bHasNext = strPicker.GetSubString(const_cast<const char**>(&pSubStr), &subLen);
        if (pSubStr && subLen > 0) {
            pSubStr[subLen] = '\0';
            ArgumentData arg(AT_STRING, pSubStr);
            pInstance->m_ArgData.PushBack(&arg);
            pInstance->m_ArgDataBackup.PushBack(&arg);
        }
    } while (bHasNext);
    pInstance->AnalyzeOutputFile();
    return pInstance;
}

const char* CLineArguments::GetArgumentString(char* pBuffer, size_t len)
{
    const char* pString = pBuffer;
    char* pCur = pBuffer;
    char* pEnd = pBuffer + len;

    for (size_t i = 0; i < m_ArgDataBackup.Count(); ++i) {
        if (pCur >= pEnd) {
            pString = NULL;
            break;
        }
        ArgumentData* pArg = reinterpret_cast<ArgumentData*>(m_ArgDataBackup.At(i));
        ASSERT(pArg);
        int len = snprintf(pCur, pEnd - pCur, "%s ", pArg->pString);
        if (len < 0 || pCur[len] != '\0') {
            pString = NULL;
            break;
        }
        pCur += len;
    }
    return pString;
}

void CLineArguments::AnalyzeOutputFile()
{
    size_t count = m_ArgData.Count();
    if (count > 1) {
        ArgumentData* pLastArg = GetArgumentAt(count - 1);
        char* pArgString = pLastArg->pString;
        if (pArgString[0] == '>' && pArgString[1] != '\0') {
            m_pFileName = pArgString + 1;
            m_ArgData.Erase(count - 1);
        }
    }
}
