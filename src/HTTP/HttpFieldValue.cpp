/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpFieldValue.h"
#include <cstring>
#include <new>
#include "HTTPBase/Token.h"
#include "HTTPBase/DateHelper.h"
#include "HttpToken.h"
#include "Common/ForwardList.h"
#include "Common/StringPicker.h"
#include "Common/PairString.h"
#include "Tracker/Trace.h"

using std::memcpy;
using std::strchr;

///////////////////////////////////////////////////////////////////////////////
//
// CHttpAcceptFieldValue Implemenation
//
//
// Accept = media-types *(parameters) [ weight *(extension) ]
///////////////////////////////////////////////////////////////////////////////
IFieldValue* CHttpAcceptFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    const char* pEnd = pString + len;
    const char* pWeightPos = CWeightFieldValue::FindPosition(pString, len);
    const char* pMediaRangeEnd = pWeightPos ? pWeightPos : pEnd;

    tMediaRange* pMediaRange = static_cast<tMediaRange*>(
                tMediaRange::CreateInstance(pString, pMediaRangeEnd - pString, buffer));
    if (!pMediaRange) {
        return NULL;
    }

    void* pMem = buffer.Malloc(sizeof(CHttpAcceptFieldValue));
    if (!pMem) {
        OUTPUT_ERROR_TRACE("Allocated Memory Failed\n");
        return NULL;
    }

    CForwardList extList(&buffer);

    // No weight and extensions.
    if (!pWeightPos) {
        return new (pMem) CHttpAcceptFieldValue(*pMediaRange, extList);
    }

    unsigned short weight = 0;
    const char* pCur = pWeightPos + 1;
    size_t weightLen = 0;
    CWeightFieldValue::StringParseResult result =
            CWeightFieldValue::GetQualityByString(pCur, pEnd - pCur, weight, weightLen);
    ASSERT(result != CWeightFieldValue::SPR_NOT_EXIST);
    if (result == CWeightFieldValue::SPR_INVALID_VALUE) {
        OUTPUT_ERROR_TRACE("INVALID Weight Value\n");
        return NULL;
    }
    pCur += weightLen;
    if (pCur == pEnd) {
        // No Accept extension.
        return new (pMem) CHttpAcceptFieldValue(*pMediaRange, extList, weight);
    }

    if (*pCur++ != ';' || pCur >= pEnd) {
        OUTPUT_ERROR_TRACE("Malformat accept string\n");
        return NULL;
    }

    while (pCur) {
        // Analyze the extensions
        const char* pDelimiter = NSCharHelper::FindChar(';', pCur, pEnd);
        const char* pExtEnd = pEnd;
        if (pDelimiter) {
            pExtEnd = pDelimiter - 1;
            if (pExtEnd < pCur) {
                OUTPUT_ERROR_TRACE("Malformat accept string\n");
                return NULL;
            }
        }
        pExtEnd = NSCharHelper::TrimRightSpace(pCur, pExtEnd);
        if (pExtEnd == NULL) {
            OUTPUT_ERROR_TRACE("Malformat accept string\n");
            return NULL;
        }

        pCur = NULL;
        if (pDelimiter) {
            pCur = NSCharHelper::TrimLeftSpace(pDelimiter + 1, pEnd);
            if (!pCur) {
                OUTPUT_ERROR_TRACE("Malformat accept string, blank extension\n");
                return NULL;
            }
        }

        tAcceptExtension* pExt = static_cast<tAcceptExtension*>(
                tAcceptExtension::CreateInstance(pCur, pExtEnd - pCur, buffer));
        if (pExt) {
            extList.PushBack(pExt);
        }
    }
    return new (pMem) CHttpAcceptFieldValue(*pMediaRange, extList, weight);
}

bool CHttpAcceptFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    size_t totalPrintLen = 0;
    size_t curPrintLen = 0;
    char* pCur = pBuffer;
    char* pEnd = pBuffer + len;
    CForwardList::Iterator extListIter = m_ExtensionList.Begin();
    CForwardList::Iterator extListIterEnd = m_ExtensionList.End();
    bool bFinished = m_MediaRange.Print(pCur, pEnd - pCur, &curPrintLen);
    pCur += curPrintLen;
    totalPrintLen += curPrintLen;
    if (pCur >= pEnd) {
        bFinished = false;
    }
    if (!bFinished) {
        goto EXIT;
    }
    bFinished = m_Weight.Print(pCur, pEnd - pCur, &curPrintLen);
    pCur += curPrintLen;
    totalPrintLen += curPrintLen;
    if (!bFinished) {
        goto EXIT;
    }
    while (extListIter != extListIterEnd) {
        if (pCur >= pEnd) {
            bFinished = false;
            break;
        }
        *pCur++ = ';';
        ++totalPrintLen;
        if (pCur >= pEnd) {
            bFinished = false;
            break;
        }
        bFinished =
            reinterpret_cast<tAcceptExtension*>(m_ExtensionList.DataAt(extListIter))->
            Print(pCur, pEnd - pCur, &curPrintLen);
        totalPrintLen += curPrintLen;
        if (!bFinished) {
            break;
        }
        pCur += curPrintLen;
        ++extListIter;
    }

EXIT:
    *pOutPrintLen = totalPrintLen;
    return bFinished;
}


///////////////////////////////////////////////////////////////////////////////
//
// CHttpTransferEncodingFieldValue Implemenation
//
///////////////////////////////////////////////////////////////////////////////
const char CHttpTransferEncodingFieldValue::s_chunkedLabel[] = "chunked";
IFieldValue* CHttpTransferEncodingFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    ValueType type;
    tTokenID token = INVALID_TOKEN_ID;
    tExtension* pExt = NULL;
    if (memcmp(pString, s_chunkedLabel, len) == 0) {
        type = VT_CHUNKED;
    } else if ((token = CTokenManager::Instance()->GetTokenID(
        CHttpTokenMap::CATEGORY_ENCODING, pString, len)) != INVALID_TOKEN_ID) {
        type = VT_ENCODING_TOKEN;
    } else {
        pExt = static_cast<tExtension*>(tExtension::CreateInstance(pString, len, buffer));
        if (!pExt) {
            OUTPUT_ERROR_TRACE("Unknown transfer-encoding string\n");
            return NULL;
        }
        type = VT_EXTENSION;
    }

    IFieldValue* pObject = NULL;
    void* pMem = buffer.Malloc(sizeof(CHttpTransferEncodingFieldValue));
    if (pMem) {
        switch (type) {
        case VT_CHUNKED:
            pObject = new (pMem) CHttpTransferEncodingFieldValue();
            break;
        case VT_ENCODING_TOKEN:
            pObject = new (pMem) CHttpTransferEncodingFieldValue(token);
            break;
        case VT_EXTENSION:
            pObject = new (pMem) CHttpTransferEncodingFieldValue(*pExt);
            break;
        default:
            ASSERT(false);
            break;
        }
    }
    return pObject;
}

bool CHttpTransferEncodingFieldValue::Print(
    char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    size_t length = 0;
    bool bFinished = false;
    switch (m_ValueType) {
    case VT_CHUNKED:
        if (sizeof(s_chunkedLabel) - 1 > len) {
            length = len;
            bFinished = false;
        } else {
            length = sizeof(s_chunkedLabel) - 1;
            bFinished = true;
        }
        memcpy(pBuffer, s_chunkedLabel, length);
        break;
    case VT_ENCODING_TOKEN:
        bFinished = m_Value.Encoding.Print(pBuffer, len, &length);
        break;
    case VT_EXTENSION:
        bFinished = m_Value.ExtensionObject.Print(pBuffer, len, &length);
        break;
    default:
        ASSERT(false);
        break;
    }

    *pOutPrintLen = length;
    return bFinished;
}


///////////////////////////////////////////////////////////////////////////////
//
// CHttpRetryAfterFieldValue Implemenation
//
///////////////////////////////////////////////////////////////////////////////
IFieldValue* CHttpRetryAfterFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    // TODO: Impl
    return NULL;
}

bool CHttpRetryAfterFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    // TODO: Impl
    ASSERT(false);
    return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CHttpAcceptLangFieldValue Implemenation
//
///////////////////////////////////////////////////////////////////////////////
IFieldValue* CHttpAcceptLangFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    // TODO: Impl
    return NULL;
}

bool CHttpAcceptLangFieldValue::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    // TODO: Impl
    ASSERT(false);
    return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CHttpSetCookieFieldValue Implemenation
//
///////////////////////////////////////////////////////////////////////////////
IFieldValue* CHttpSetCookieFieldValue::CreateInstance(
        const char* pString, size_t len, CLazyBuffer& buffer)
{
    CHttpSetCookieFieldValue* pInstance = NULL;
    CStringPicker stringPicker(';', pString, pString + len);
    const char* subString = NULL;
    size_t subStringLen = 0;
    bool bHasNext = stringPicker.GetSubString(&subString, &subStringLen);
    char* pName = NULL;
    char* pValue = NULL;
    if (subString &&
        CreateNameValue(subString, subStringLen, buffer, &pName, &pValue)) {
        tSecondTick expireTime = 0;
        char* pPath = NULL;
        char* pDomain = NULL;
        bool bSecure = false;
        while (bHasNext) {
            bHasNext = stringPicker.GetSubString(&subString, &subStringLen);
            if (subString == NULL) {
                break;
            }
            AttributeValue value;
            CookieAttrID attrID =
                ParseAttribute(subString, subStringLen, buffer, &value);
            switch (attrID) {
            case COOKIE_DOMAIN:
                pDomain = value.pString;
                break;
            case COOKIE_PATH:
                pPath = value.pString;
                break;
            case COOKIE_EXPIRES:
            case COOKIE_MAX_AGE:
                if (expireTime == 0) {
                    expireTime = value.ExpiredTime;
                }
                break;
            case COOKIE_SECURE:
                bSecure = true;
                break;
            case COOKIE_COMMENT:
            case COOKIE_COMMENT_URI:
            case COOKIE_VERSION:
            case COOKIE_DISCARD:
            case COOKIE_PORT:
                break;
            default:
                OUTPUT_WARNING_TRACE("Wrong Cookie attribute: %s\n", subString);
                return NULL;
            }
        }
        void* pMem = buffer.Malloc(sizeof(CHttpSetCookieFieldValue));
        if (pMem) {
            pInstance = new (pMem) CHttpSetCookieFieldValue(
                pDomain, pPath, pName, pValue, expireTime, bSecure, false);
        }
    }
    return pInstance;
}

bool CHttpSetCookieFieldValue::Print(
    char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    const char* pDomain = m_CookieValue.GetDomainName();
    const CHttpCookieData::Attribute* pCookieAttr = m_CookieValue.GetAttribute();
    int outputLen = snprintf(
        pBuffer,
        len,
        "\n\tDomain: %s\n"
        "\tName: %s\n"
        "\tValue: %s\n"
        "\tPath: %s\n"
        "\tExpire: %ld\n"
        "\tSecure: %s\n"
        "\tWild Matched: %s\n",
        pDomain,
        pCookieAttr->pName,
        pCookieAttr->pValue,
        pCookieAttr->pPath,
        pCookieAttr->ExpireTime,
        pCookieAttr->bSecure ? "TRUE" : "FALSE",
        pCookieAttr->bWildcardMatched ? "TRUE" : "FALSE");

    if (outputLen > 0) {
        *pOutPrintLen = outputLen;
        // Note: We never know if the output is complete if use snprintf
        // since we don't known how many bytes is truncated, if the last null terminator
        // has been truncated then the string output is complete but the result from snprintf
        // is still truncated.
        return true;
    }
    *pOutPrintLen = 0;
    return false;
}

bool CHttpSetCookieFieldValue::CreateNameValue(
    const char* pString,
    size_t len,
    CLazyBuffer& buffer,
    char** pOutName, char** pOutValue)
{
    bool bRes = false;
    size_t nameLen = 0;
    size_t valueLen = 0;
    CPairString nameValuePair('=', pString, len);
    const char* pName = nameValuePair.GetFirst(&nameLen);
    const char* pValue = nameValuePair.GetSecond(&valueLen);
    if (pName && pValue) {
        *pOutName = const_cast<char*>(buffer.StoreNString(pName, nameLen));
        *pOutValue = const_cast<char*>(buffer.StoreNString(pValue, valueLen));
        if (*pOutName && *pOutValue) {
            bRes = true;
        }
    }
    return bRes;
}

CookieAttrID CHttpSetCookieFieldValue::ParseAttribute(
    const char* pString,
    size_t len,
    CLazyBuffer& buffer,
    AttributeValue* pOutValue)
{
    CPairString nameValuePair('=', pString, len);
    size_t nameLen = 0;
    size_t valueLen = 0;
    const char* pName = nameValuePair.GetFirst(&nameLen);
    const char* pValue = nameValuePair.GetSecond(&valueLen);
    memset(pOutValue, 0, sizeof(*pOutValue));
    if (pName == NULL) {
        return COOKIE_INVALID;
    }

    int attrToken = COOKIE_INVALID;
    if ((attrToken = CTokenManager::Instance()->GetTokenID(
        CHttpTokenMap::CATEGORY_COOKIE_ATTR_ID, pName, nameLen)) != INVALID_TOKEN_ID) {
        if (attrToken == COOKIE_SECURE) {
            return nameValuePair.IsPair() ? COOKIE_INVALID : COOKIE_SECURE;            
        }
        if (!nameValuePair.IsPair() || pValue == NULL) {
            return COOKIE_INVALID;
        }

        struct tm date;
        if (attrToken == COOKIE_EXPIRES) {
            // GMT string format
            if (!CDateHelper::GetCookieDateByString(
                pValue, valueLen, &date)) {
                OUTPUT_WARNING_TRACE("Wrong cookie expire date: %s\n", pValue);
                return COOKIE_INVALID;
            }
            CDateHelper::GMTDate2SecondTickets(&date, &pOutValue->ExpiredTime);
        } else if (attrToken == COOKIE_MAX_AGE) {
            // Seconds format.
            time_t now;
            time(&now);
            pOutValue->ExpiredTime = atoll(pValue) + now;
        } else if (attrToken == COOKIE_DOMAIN) {
            pOutValue->pString = const_cast<char*>(buffer.StoreNString(pValue, valueLen));
        } else if (attrToken == COOKIE_PATH) {
            if (*pValue == '/') {
                pOutValue->pString = const_cast<char*>(buffer.StoreNString(pValue, valueLen));
            } else {
                OUTPUT_WARNING_TRACE("Wrong cookie path: %s\n", pValue);
                attrToken = COOKIE_INVALID;
            }
        }
    } else {
        // Invalid value.
        attrToken = COOKIE_INVALID;
    }
    return static_cast<CookieAttrID>(attrToken);
}


///////////////////////////////////////////////////////////////////////////////
//
// CHttpCookieFieldValue Implemenation
//
// Cookie: name=value
///////////////////////////////////////////////////////////////////////////////
bool CHttpCookieFieldValue::Print(
    char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    size_t freespaceLen = len;
    size_t nameLen = strlen(m_pValue->pName);
    size_t copiedLen = (freespaceLen >= nameLen) ? nameLen : freespaceLen;
    freespaceLen -= copiedLen;
    memcpy(pBuffer, m_pValue->pName, copiedLen);
    *pOutPrintLen = copiedLen;
    if (freespaceLen == 0) {
        return false;
    }
    pBuffer[copiedLen] = '=';
    --freespaceLen;
    ++*pOutPrintLen;
    if (freespaceLen == 0) {
        return false;
    }

    bool bFinished = false;
    size_t valueLen = strlen(m_pValue->pValue);
    if (freespaceLen >= valueLen) {
        bFinished = true;
        copiedLen = valueLen;
    } else {
        copiedLen = freespaceLen;
    }
    freespaceLen -= copiedLen;
    memcpy(pBuffer + *pOutPrintLen, m_pValue->pValue, copiedLen);
    *pOutPrintLen += copiedLen;
    return bFinished;
}
