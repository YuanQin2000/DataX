/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "FieldValue.h"

#include <new>
#include <cstdio>
#include <cctype>

#include "Common/ErrorNo.h"
#include "Common/CharHelper.h"
#include "DateHelper.h"

using std::snprintf;
using std::isdigit;

///////////////////////////////////////////////////////////////////////////////
//
// Common/Base Field Value Class Implementations
//
///////////////////////////////////////////////////////////////////////////////

IFieldValue* CStringFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    char* pStr = reinterpret_cast<char*>(
            buffer.Malloc(len + 1 + sizeof(CStringFieldValue)));
    if (!pStr) {
        return NULL;
    }
    memcpy(pStr , pString, len);
    pStr[len] = '\0';
    return new (pStr + len + 1) CStringFieldValue(pStr, static_cast<int>(len));
}

bool CStringFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    bool bFinished = false;
    size_t copyLen = len;
    if (len >= m_Length) {
        bFinished = true;
        copyLen = m_Length;
    }
    memcpy(pBuffer, m_pString, copyLen);
    *pOutPrintLen = copyLen;
    return bFinished;
}


IFieldValue* CIntFieldValue::CreateInstance(
    const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    int number;
    IFieldValue* pValue = NULL;
    if (NSCharHelper::GetIntByString(pString, len, &number)) {
        void* pMem = buffer.Malloc(sizeof(CIntFieldValue));
        if (pMem) {
            pValue = new (pMem) CIntFieldValue(number);
        }
    }
    return pValue;
}


bool CWeightFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    char* pCur = pBuffer;
    char* pEnd = pBuffer + len;
    size_t length = 0;

    *pOutPrintLen = 0;
    if (m_Quality == 0) {
        return true;
    }
    if (len == 0) {
        return false;
    }

    // Avoid using the float computing.
    static const char* s_pQValue = ";q=";
    const char* pPreCur = s_pQValue;

    while (pCur < pEnd) {
        if (!*pPreCur) {
            break;
        }
        *pCur++ = *pPreCur++;
    }
    length = pCur - pBuffer;

    if (pCur == pEnd) {
        *pOutPrintLen = length;
        return false;
    }

    ++length;
    if (m_Quality == 1000) {
        *pCur = '1';
        *pOutPrintLen = length;
        return true;
    }
    *pCur++ = '0';
    if (pCur == pEnd) {
        *pOutPrintLen = length;
        return false;
    }
    *pCur++ = '.';
    ++length;

    int base = 100;
    unsigned short qualityValue = m_Quality;
    while (pCur < pEnd) {
        if (qualityValue == 0) {
            break;
        }
        unsigned char ch = qualityValue / base;
        *pCur = ch + '0';
        qualityValue %= base;
        base /= 10;
        ++length;
        ++pCur;
    }
    *pOutPrintLen = length;
    return qualityValue == 0;
}


IFieldValue* CWeightFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    unsigned short quality;
    size_t consumedLen = 0;
    IFieldValue* pValue = NULL;
    if (GetQualityByString(pString, len, quality, consumedLen) == SPR_OK) {
        if (consumedLen == len) {
            void* pMem = buffer.Malloc(sizeof(CIntFieldValue));
            if (pMem) {
                pValue = new (pMem) CIntFieldValue(quality);
            }
        }
    }
    return pValue;
}

CWeightFieldValue::StringParseResult CWeightFieldValue::GetQualityByString(
    const char* pString,
    size_t len,
    unsigned short& quality,
    size_t& consumedLen)
{
    ASSERT(len > 0);

    const char* pCur = pString;
    const char* pEnd = pString + len;

    while (pCur < pEnd) {
        if (!isspace(*pCur)) {
            break;
        }
        ++pCur;
    }
    if (pCur == pEnd || *pCur != 'q') {
        return SPR_NOT_EXIST;
    }
    ++pCur;
    if (pCur == pEnd || *pCur != '=') {
        return SPR_NOT_EXIST;
    }
    ++pCur;
    if (pCur == pEnd) {
        return SPR_NOT_EXIST;
    }

    unsigned short qNum = 0;
    if (*pCur == '0') {
        ++pCur;
        if (pCur == pEnd || *pCur != '.') {
            return SPR_INVALID_VALUE;
        }
        ++pCur;

        unsigned short factor = 1000; 
        while (pCur < pEnd) {
            char ch = *pCur;
            if (!isdigit(ch)) {
                break;
            }
            qNum = qNum * 10 + ch - '0';
            factor /= 10;
            ++pCur;
        }
        qNum *= factor;
    } else if (*pCur == '1') {
        qNum = 1000;
        if (isdigit(pCur[1])) {
            return SPR_INVALID_VALUE;
        }
    }

    if (qNum == 0) {
        return SPR_INVALID_VALUE;
    }

    if (pCur == pEnd) {
        consumedLen = pEnd - pString;
    } else {
        ASSERT(pCur < pEnd);
        const char* pNextChar = NSCharHelper::TrimLeftSpace(pCur + 1, pEnd);
        consumedLen = pNextChar ? pNextChar - pString : pEnd - pString;
    }
    quality = qNum;
    return SPR_OK;
}

const char* CWeightFieldValue::FindPosition(const char* pString, size_t len)
{
    ASSERT(len > 0);

    const char* pEnd = pString + len;
    const char* pPos = NULL;
    const char* pDelimiter = NSCharHelper::FindChar(';', pString, pEnd);
    while (pDelimiter) {
        const char* pStart = pDelimiter;
        bool bFoundQ = false;
        while (++pStart < pEnd) {
            char ch = *pStart;
            if (bFoundQ) {
                if (ch == '=') {
                    // Found.
                    pPos = pDelimiter;
                }
                break;
            }

            if (ch == 'q') {
                bFoundQ = true;
            } else if (!isspace(ch)) {
                break;
            }
        }

        const char* pNext = pDelimiter + 1;
        if (pPos || pNext >= pEnd) {
            break;
        }
        pDelimiter = NSCharHelper::FindChar(';', pNext, pEnd);
    }
    return pPos;
}


IFieldValue* CDateFieldValue::CreateInstance(
    const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    struct tm tmObj;
    IFieldValue* pValue = NULL;
    if (CDateHelper::GetHTTPDateByString(pString, len, &tmObj)) {
        tSecondTick secTick = 0;
        if (CDateHelper::GMTDate2SecondTickets(&tmObj, &secTick)) {
            void* pMem = buffer.Malloc(sizeof(CDateFieldValue));
            if (pMem) {
                pValue = new (pMem) CDateFieldValue(secTick);
            }
        }
    }
    return pValue;
}

bool CDateFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    return CDateHelper::Serialize(pBuffer, len, m_Value, pOutPrintLen);
}


IFieldValue* CProductFieldValue::CreateInstance(const char* pString,
                                                size_t len,
                                                CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    const char* pVersion = NULL;
    char* pName = NULL;
    uint8_t* pMem = reinterpret_cast<uint8_t*>(
        buffer.Malloc(len + 1 + sizeof(CProductFieldValue)));
    if (!pMem) {
        return NULL;
    }
    pName = reinterpret_cast<char*>(pMem);
    memcpy(pName, pString, len);
    pName[len] = '\0';
    char* pNameEnd = const_cast<char*>(NSCharHelper::FindChar('/', pName, pName + len));
    if (pNameEnd) {
        *pNameEnd = '\0';
        pVersion = pNameEnd + 1;
    }
    return new (pMem + len + 1) CProductFieldValue(pName, pVersion);
}

bool CProductFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    size_t copied;
    size_t outputLen = 0;
    bool bFinished = NSCharHelper::CopyNChars(pBuffer, len, m_pName, &copied);
    outputLen = copied;
    len -= copied;
    if (bFinished && m_pVersion && len > 0) {
        bFinished = false;
        pBuffer[copied] = '/';
        --len;
        ++outputLen;
        if (len > 0) {
            bFinished = NSCharHelper::CopyNChars(pBuffer + outputLen, len, m_pVersion, &copied);
        }
        *pOutPrintLen = outputLen + copied;
    }
    return bFinished;
}
