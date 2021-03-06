/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CharHelper.h"
#include <cstring>
#include <strings.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Tracker/Trace.h"

using std::memcpy;
using std::memcmp;
using std::strlen;
using std::strncmp;
using std::strstr;
using std::malloc;
using std::snprintf;

static const char* SimpleFindSubStr(
    const char* pSubStart,
    size_t subLen,
    const char* pString,
    const char* pEnd,
    bool bReturnEndIfNotFound)
{
    const char* pCur = pString;
    const char* pFoundPos = NULL;
    if (pEnd) {
        while (pCur + subLen <= pEnd) {
            if (memcmp(pCur, pSubStart, subLen) == 0) {
                pFoundPos = pCur;
                break;
            }
            ++pCur;
        }
    } else {
        while (*pCur != '\0') {
            if (strncmp(pCur, pSubStart, subLen) == 0) {
                pFoundPos = pCur;
                break;
            }
            ++pCur;
        }
    }
    if (bReturnEndIfNotFound && pFoundPos == NULL) {
        pFoundPos = pEnd ? pEnd : pCur;
    }
    return pFoundPos;
}

/**
 * Calculate the max common matched length of the prefix and postfix sub string.
 * This max length will determinate that how long step the pattern string should be move forward.
 * This is the key concept of the KMP algorithm.
 * 
 * Here is the key point of the max common matched length.
 * 1) P1 P2 P3, ..., P(k-1), P(k), ..., P(j-k), P(j-k+1), ..., P(j)
 * 
 * 
 * @param pString The pattern string.
 * @param pArray  The buffer to store the max common matched length data.
 * @param len The length of the string and the array.
 */
static void KMPCalcMaxCommonLengthTable(
    const char* pString, uint16_t* pArray, size_t len)
{
    uint16_t j = 1;
    uint16_t k = 0;

    pArray[0] = 0;
    while (j < len - 1) {
        if (pString[j] == pString[k]) {  
            pArray[j++] = ++k;
        } else {
            k = pArray[k];
            if (k == 0) {
                pArray[j++] = 0;
            }
        }
    }
    pArray[len - 1] = 0;    // Not used.
}

static const char* KMPFindSubStr(
    const char* pSubStart,
    size_t subLen,
    const char* pString,
    const char* pEnd,
    bool bReturnEndIfNotFound)
{
    ASSERT(subLen > 2, "Avoid using KMP when the sub string is shorter than 3 node.\n");

    uint16_t* pMCLTable = NULL;
    bool bNeedFree = false;
    if (subLen <= 256) {
        pMCLTable = VAR_ARRAY(uint16_t, subLen);
    } else if (subLen <= 0xFFFF) {
        pMCLTable = reinterpret_cast<uint16_t*>(malloc(sizeof(uint16_t) * subLen));
        if (pMCLTable == NULL) {
            OUTPUT_ERROR_TRACE(
                "Malloc %d Bytes memory failed.\n", subLen * sizeof(uint16_t));
            return NULL;
        }
        bNeedFree = true;
    } else {
        ASSERT(false, "Compared sub string is too long: %d\n", subLen);
        return NULL;
    }

    KMPCalcMaxCommonLengthTable(pSubStart, pMCLTable, subLen);

    const char* pCur = pString;
    const char* pSubCur = pSubStart;
    const char* pSubEnd = pSubStart + subLen;
    const char* pFoundPos = NULL;
    const char* pCompareStart = pCur;
    if (pEnd) {
        size_t remainLen = subLen;
        while (pCur + remainLen <= pEnd && pSubCur < pSubEnd) {
            if (*pCur != *pSubCur) {
                if (pSubCur > pSubStart) {
                    size_t partialMatchLen = pMCLTable[pSubCur - pSubStart - 1];
                    pSubCur = pSubStart + partialMatchLen;
                    pCompareStart = pCur - partialMatchLen;
                    remainLen = pSubEnd - pSubCur;
                } else {
                    ++pCur;
                    ++pCompareStart;
                }
            } else {
                ++pCur;
                ++pSubCur;
                --remainLen;
            }
        }
    } else {
        char ch = *pCur;
        while (ch != '\0' && pSubCur < pSubEnd) {
            if (ch != *pSubCur) {
                if (pSubCur > pSubStart) {
                    size_t partialMatchLen = pMCLTable[pSubCur - pSubStart - 1];
                    pSubCur = pSubStart + partialMatchLen;
                    pCompareStart = pCur - partialMatchLen;
                } else {
                    ++pCur;
                    ++pCompareStart;
                }
            } else {
                ++pCur;
                ++pSubCur;
            }
            ch = *pCur;
        }
    }

    if (bNeedFree) {
        free(pMCLTable);
    }

    if (pSubCur == pSubEnd) {
        // Found.
        pFoundPos = pCompareStart;
    } else if (bReturnEndIfNotFound) {
        pFoundPos = pEnd ? pEnd : pCur;
    }
    return pFoundPos;
}


namespace NSCharHelper
{

bool GetIntByString(const char* pString, size_t len, int* pOutNum)
{
    ASSERT(pString);
    ASSERT(pOutNum);

    int n = 0;
    size_t i = 0;
    bool negative = false;
    if (pString[0] == '-') {
        negative = true;
        ++i;
    }
    while (i < len) {
        char ch = pString[i];
        if (!isdigit(ch)) {
            return false;
        }
        n = n * 10 + ch - '0';
        ++i;
    }
    if (negative && n != 0) {
        n *= -1;
    }
    *pOutNum = n;
    return true;
}

bool GetStringByInt(
    int number, char* pBuffer, size_t buflen, size_t* pOutStringLen)
{
    ASSERT(pBuffer);
    ASSERT(buflen > 0);

    size_t len = 0;
    int num = number;
    if (num < 0) {
        pBuffer[len++] = '-';
        num *= -1;
    }

    bool bCompleted = true;
    while (num != 0) {
        pBuffer[len++] = (num % 10) + '0';
        num /= 10;
        if (len == buflen) {
            bCompleted = false;
            break;
        }
    }

    char* pBegin = number >= 0 ? pBuffer : pBuffer + 1;
    char* pEnd = pBuffer + len - 1;
    while (pBegin < pEnd) {
        char tmp = *pBegin;
        *pBegin = *pEnd;
        *pEnd = tmp;
        ++pBegin;
        --pEnd;
    }

    if (pOutStringLen) {
        *pOutStringLen = len;
    }
    return bCompleted;
}

const char* FindSubStr(
    const char* pSub,
    size_t subLen,
    const char* pString,
    const char* pEnd /* = NULL */,
    bool bReturnEndIfNotFound /* = false */)
{
    ASSERT(pSub && subLen > 0);
    ASSERT(pString);

    if (subLen == 1) {
        return FindChar(*pSub, pString, pEnd, bReturnEndIfNotFound);
    }
    if (subLen == 2) {
        return SimpleFindSubStr(
            pSub, subLen, pString, pEnd, bReturnEndIfNotFound);
    }
    return KMPFindSubStr(pSub, subLen, pString, pEnd, bReturnEndIfNotFound);
}

const char* FindChar(
    char target,
    const char* pString,
    const char* pEnd /* = NULL */,
    bool bReturnEndIfNotFound /* = false */)
{
    ASSERT(pString);

    const char* pCur = pString;
    const char* pFound = NULL;
    if (pEnd) {
        while (pCur < pEnd) {
            if (*pCur == target) {
                pFound = pCur;
                break;
            }
            ++pCur;
        }
    } else {
        char ch = *pCur;
        while (ch != '\0') {
            if (ch == target) {
                pFound = pCur;
                break;
            }
            ++pCur;
            ch = *pCur;
        }
    }
    if (bReturnEndIfNotFound && pFound == NULL) {
        pFound = pCur;
    }
    return pFound;
}

bool CopyNChars(char* pDst, size_t dstLen, const char* pSrc, size_t* pOutLen)
{
    ASSERT(dstLen > 0);
    ASSERT(pOutLen);

    size_t copied = 0;
    while (dstLen > 0 && *pSrc != '\0') {
        *pDst++ = *pSrc++;
        ++copied;
        --dstLen;
    }
    *pOutLen = copied;
    return *pSrc == '\0';
}

bool IsHexChar(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

char* Trim(char* pString)
{
    ASSERT(pString);

    char* pBegin = pString;
    char ch;
    while ((ch = *pBegin)) {
        if (isprint(ch) && !isspace(ch)) {
            break;
        }
        ++pBegin;
    }
    char* pCur = pBegin;
    char* pEnd = NULL;
    while ((ch = *pCur)) {
        if (!isprint(ch) || isspace(ch)) {
            pEnd = pCur;
        } else {
            pEnd = NULL;
        }
        ++pCur;
    }
    if (pEnd) {
        *pEnd = '\0';
    }
    return *pBegin == '\0' ? NULL : pBegin;
}

const char* Trim(const char* pString, size_t len, size_t* pOutLen)
{
    ASSERT(pString);
    ASSERT(pOutLen);

    const char* pBegin = pString;
    char ch;
    size_t i = 0;
    size_t outLen = len;
    while (i < len) {
        ch = *pBegin;
        if (isprint(ch) && !isspace(ch)) {
            break;
        }
        ++pBegin;
        ++i;
    }
    outLen -= i;
    i = 0;

    const char* pCur = pBegin;
    const char* pEnd = NULL;
    while (i < outLen) {
        ch = *pCur;
        if (!isprint(ch) || isspace(ch)) {
            pEnd = pCur;
        } else {
            pEnd = NULL;
        }
        ++pCur;
        ++i;
    }
    if (pEnd) {
        outLen = pEnd - pBegin;
    }
    *pOutLen = outLen;
    return pBegin;
}

const char* TrimLeftSpace(
    const char* pString, const char* pEnd /* = NULL */)
{
    const char* pRes = NULL;
    if (pEnd == NULL) {
        char ch = *pString;
        while (ch != '\0') {
            if (!isspace(ch)) {
                pRes = pString;
                break;
            }
            ch = *++pString;
        }
    } else {
        char ch = *pString;
        while (ch != '\0' && pString < pEnd) {
            if (!isspace(ch)) {
                pRes = pString;
                break;
            }
            ch = *++pString;
        }
    }
    return pRes;
}

char* TrimRightSpace(char* pString)
{
    char* pNewEnd = pString + strlen(pString);
    while (pString < pNewEnd) {
        if(!isspace(*--pNewEnd)) {
            *(pNewEnd + 1) = '\0';
            break;
        }
    }
    return pString < pNewEnd ? pString : NULL;
}

const char* TrimRightSpace(const char* pBegin, const char* pEnd)
{
    const char* pRes = NULL;
    while (pBegin < pEnd) {
        if (!isspace(*(pEnd - 1))) {
            pRes = pEnd;
            break;
        }
        --pEnd;
    }
    return pRes;
}

bool IsBlankString(const char* pString)
{
    char ch = '\0';
    while ((ch = *pString++) != '\0') {
        if (!isspace(ch)) {
            return false;
        }
    }
    return true;
}

bool StringCompare(const char* pString1, const char* pString2)
{
    return strcmp(pString1, pString2) < 0;
}

bool StringCaseCompare(const char* pString1, const char* pString2)
{
    return strcasecmp(pString1, pString2) < 0;
}

int StringCompare1(
    const char* pStr1,
    size_t len1,
    const char* pStr2,
    size_t len2,
    bool bCaseSensitive)
{
    size_t n = 0;
    if (len1 == 0) {
        if (len2 == 0) {
            return bCaseSensitive ?
                strcmp(pStr1, pStr2) : strcasecmp(pStr1, pStr2);
        }
        // len2 != 0
        n = len2;
    } else if (len2 == 0 || len1 == len2) {
         // len1 != 0
        n = len1;
    } else {
        // len1 != 0 && len2 != 0 && len1 != len2
        size_t min = len1 < len2 ? len1 : len2;
        int res = bCaseSensitive ?
            strncmp(pStr1, pStr2, min) : strncasecmp(pStr1, pStr2, min);
        return res != 0 ? res : len1 - len2;
    }
    return bCaseSensitive ?
        strncmp(pStr1, pStr2, n) : strncasecmp(pStr1, pStr2, n);
}

char* String2LowerCase(char* pString)
{
    char* pCur = pString;
    char ch = *pCur;
    while (ch != '\0') {
        if (ch >= 'A' && ch <= 'Z') {
            ch += ('a' - 'A');
            *pCur = ch;
        }
        ch = *++pCur;
    }
    return pString;
}

size_t StringNLength(const char* pStr, size_t len)
{
    const char* pCur = pStr;
    const char* pEnd = pCur + len;
    while (*pCur != '\0' && pCur < pEnd) {
        ++pCur;
    }
    if (pCur < pEnd) {  // add the null terminator
        ++pCur;
    }
    return pCur - pStr;
}

void Reverse(char* pString, size_t len /* = 0 */)
{
    size_t strLen = len;
    if (strLen == 0) {
        strLen = strlen(pString);
    }
    ASSERT(strLen);
    char* pFront = pString;
    char* pTail = pString + strLen;
    while (pFront > pTail) {
        char temp = *pFront;
        *pFront = *pTail;
        *pTail = temp;
        ++pFront;
        --pTail;
    }
}

};
