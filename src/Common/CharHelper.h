/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_CHAR_HELPER_H__
#define __COMMON_CHAR_HELPER_H__

#include <cstring>

#include "Common/Typedefs.h"

using std::strlen;

namespace NSCharHelper
{

/**
 * @brief Parase the digit string and return the integer
 *        same as atoi but the string is terminated by the special length.
 * @param pString the string to be parse.
 * @param len the string length
 * @param pOutNum output parameter for store the number value.
 * @return True if parse OK, false otherwise.
 */
bool GetIntByString(const char* pString, size_t len, int* pOutNum);

// Conver int to the digit string.
bool GetStringByInt(int number, char* pBuffer, size_t buflen, size_t* pOutStringLen);

/**
 * Find the first sub string in the string (null terminated or not)
 * @param pSub The sub string
 * @param targetLen the sub string length (not contain the null terminator if has)
 * @param pString The searched string.
 * @param pEnd The end position (not belong to the content) of the searched string if not NULL
 * @param bReturnEndIfNotFound Flag to control the return value.
 * @return Return the position if found otherwise NULL or the End position of the string.
 */
const char* FindSubStr(
    const char* pSub,
    size_t subLen,
    const char* pString,
    const char* pEnd = NULL,
    bool bReturnEndIfNotFound = false);

/**
 * Find the first specific character in the string (null terminated or not)
 * @param target The character
 * @param pString The searched string.
 * @param pEnd The end position (not belong to the content) of the searched string if not NULL
 * @param bReturnEndIfNotFound Flag to control the return value.
 * @return Return the position if found otherwise NULL or the End position of the string.
 */
const char* FindChar(
    char target,
    const char* pString,
    const char* pEnd = NULL,
    bool bReturnEndIfNotFound = false);

/**
 * Copy at most dstLen character from pSrc to pDst.
 * @param pDst buffer for the characters copied destination.
 * @param dstLen buffer length
 * @param pSrc source character string
 * @param pOutLen Output parameter, store the value how many character copied actually.
 * @return true if the string has been copied completely.
 * @attention NOT copy the end-null character
 */
bool CopyNChars(char* pDst, size_t dstLen, const char* pSrc, size_t* pOutLen);

bool IsHexChar(char ch);
char* Trim(char* pString);

/**
 * Trim the constant string (non-writable).
 * @param pString The string to trim (not null terminated)
 * @param len the string length.
 * @param pOutLen The string length after the the trim.
 * @return return the string address after the trim.
 */
const char* Trim(const char* pString, size_t len, size_t* pOutLen);

/**
 * @brief Trim the left side of the string (front side)
 * @param pString. The string to be trimmed
 * @param pEnd, The end of the string, null if not specified (terminated with '\0')
 * @return return the started position of the trimmed string.
 */
const char* TrimLeftSpace(const char* pString, const char* pEnd = NULL);

/**
 * @brief Trim the string at the end side,
 *        will change the parameter by replace the ended space with '\0'
 * @return return the string if not blank.
 */
char* TrimRightSpace(char* pString);

/**
 * @brief Trim the string on the end side
 * @param pBegin, the string begin position
 * @param pEnd, the string end position, @note, the string content doesn't include the pEnd.
 * @return return the trim position on the right if not blank, otherwise NULL
 */
const char* TrimRightSpace(const char* pBegin, const char* pEnd);

bool IsBlankString(const char* pString);
inline bool IsEmptyString(const char* pString)
{
    return *pString == '\0';
}
bool StringCompare(const char* pString1, const char* pString2);
bool StringCaseCompare(const char* pString1, const char* pString2);
char* String2LowerCase(char* pString);

int StringCompare1(
    const char* pStr1,
    size_t len1,
    const char* pStr2,
    size_t len2,
    bool bCaseSensitive);

/**
 * @brief Calculate the c style string length, contained the '\0' terminator if has
 * @param pStr Calculated string
 * @param len Indicate the max character can be checked.
 * @return length of string, less or, equal len (if no '\0')
 */
size_t StringNLength(const char* pStr, size_t len);

/**
 * @brief Reverse string order.
 * @param pString the string to be reversed
 * @param len the string length, may not specified (len == 0)
 */
void Reverse(char* pString, size_t len = 0);

};

#endif