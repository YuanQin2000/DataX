/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_FIELD_VALUE_H__
#define __HTTP_BASE_FIELD_VALUE_H__

#include <cstdio>
#include <cstring>
#include <new>
#include <sys/time.h>
#include "Token.h"
#include "Memory/LazyBuffer.h"
#include "Common/StringIndex.h"
#include "Common/CharHelper.h"
#include "Common/ForwardList.h"
#include "Tracker/Trace.h"

using std::strlen;
using std::strchr;
using std::memcpy;
using std::snprintf;

#define DECLARE_STATIC_CREATE_FUNCTION \
public: static IFieldValue* CreateInstance( \
    const char* pString, size_t len, CLazyBuffer& buffer)


class IFieldValue
{
public:
    virtual ~IFieldValue() {}
    virtual bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// Common/Base Field Value Class Definitions
//
///////////////////////////////////////////////////////////////////////////////

class CStringFieldValue : public IFieldValue
{
public:
    CStringFieldValue(const char* pValue, int len = -1) :
        IFieldValue(),
        m_pString(pValue)
    {
        ASSERT(pValue);
        m_Length = (len == -1) ? strlen(pValue) : len;
    }
    ~CStringFieldValue() {}

    const char* Value() const { return m_pString; }
    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    const char* m_pString;  // Not owned
    size_t m_Length;
};


class CIntFieldValue : public IFieldValue
{
public:
    CIntFieldValue(int n) : IFieldValue(), m_Value(n) {}
    ~CIntFieldValue() {}

    int Value() const { return m_Value; }
    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
    {
        return NSCharHelper::GetStringByInt(m_Value, pBuffer, len, pOutPrintLen);
    }

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    const int m_Value;
};


class CWeightFieldValue : public IFieldValue
{
public:
    enum StringParseResult {
        SPR_OK,
        SPR_NOT_EXIST,
        SPR_INVALID_VALUE
    };

public:
    CWeightFieldValue(unsigned short qValue = 0) :
        IFieldValue(),
        m_Quality(qValue) { ASSERT(qValue <= 1000); }
    ~CWeightFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    static StringParseResult GetQualityByString(
        const char* pString,
        size_t len,
        unsigned short& quality,
        size_t& consumedLen);
    static const char* FindPosition(const char* pString, size_t len);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    unsigned short m_Quality;     // divide 1000
};


class CDateFieldValue : public IFieldValue
{
public:
    CDateFieldValue(tSecondTick tv) : m_Value(tv) {}
    ~CDateFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    tSecondTick m_Value;
};


class CProductFieldValue : public IFieldValue
{
public:
    CProductFieldValue(const char* pName, const char* pVersion = NULL) :
        m_pName(pName),
        m_pVersion(pVersion) { ASSERT(pName); }
    ~CProductFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    const char* m_pName;
    const char* m_pVersion;
};


template <int TOKEN>
class CTokenIndexFieldValue : public IFieldValue
{
public:
    CTokenIndexFieldValue(const CTokenIndexFieldValue& rhs) :
        IFieldValue(),
        m_Index(rhs.m_Index) {}
    CTokenIndexFieldValue(tTokenID index = 0) :
        IFieldValue(),
        m_Index(index) {}
    ~CTokenIndexFieldValue() {}

    tTokenID GetToken() const { return m_Index; }
    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    tTokenID m_Index;
};

template <int TOKEN>
bool CTokenIndexFieldValue<TOKEN>::Print(
    char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    if (m_Index < 0) {
        return 0;
    }

    const char* pString = CTokenManager::Instance()->
        GetTokenMapInstance()->GetTokenString(TOKEN, m_Index);
    size_t i = 0;
    bool bComplete = false;
    while (i < len) {
        if (*pString == '\0') {
            bComplete = true;
            break;
        }
        pBuffer[i++] = *pString++;
    }
    *pOutPrintLen = i;
    return bComplete;
}

template <int TOKEN>
IFieldValue* CTokenIndexFieldValue<TOKEN>::CreateInstance(
    const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    int index = INVALID_TOKEN_ID;
    IFieldValue* pValue = NULL;
    if ((index = CTokenManager::Instance()->GetTokenID(
            TOKEN, pString, len)) != INVALID_TOKEN_ID) {
        ASSERT(index >= 0);
        void* pMem = buffer.Malloc(sizeof(CTokenIndexFieldValue<TOKEN>));
        if (pMem) {
            pValue = new (pMem) CTokenIndexFieldValue<TOKEN>(index);
        }
    } else {
        OUTPUT_ERROR_TRACE("Get Token ID from Category %d failed\n", TOKEN);
    }
    return pValue;
}


template <int TOKEN>
class CTokenIndexWeightFieldValue : public IFieldValue
{
public:
    CTokenIndexWeightFieldValue(size_t index, unsigned short weight = 0) :
        IFieldValue(),
        m_TokenIndexObject(index),
        m_WeightObject(weight) {}

    ~CTokenIndexWeightFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    CTokenIndexFieldValue<TOKEN> m_TokenIndexObject;
    CWeightFieldValue m_WeightObject;
};

template <int TOKEN>
bool CTokenIndexWeightFieldValue<TOKEN>::Print(
    char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    size_t outputLen = 0;
    bool bFinished = false;
    size_t length = 0;
    if (m_TokenIndexObject.Print(pBuffer, len, &length)) {
        char* pCur = pBuffer + length;
        len -= length;
        outputLen = length;
        if (len > 0) {
            bFinished = m_WeightObject.Print(pCur, len, &length);
            outputLen += length;
        }
    }
    *pOutPrintLen = outputLen;
    return bFinished;
}

template <int TOKEN>
IFieldValue* CTokenIndexWeightFieldValue<TOKEN>::
CreateInstance(const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    int tokenIndex = -1;
    unsigned short weight = 0;
    const char* pQualityString = NULL;
    const char* pEnd = pString + len;
    IFieldValue* pValue = NULL;
    const char* pAnchor = NSCharHelper::FindChar(';', pString, pEnd);
    const char* pTokenEnd = pEnd;
    if (pAnchor) {
        pTokenEnd = NSCharHelper::TrimRightSpace(pString, pAnchor);
        if (pTokenEnd == NULL) {
            OUTPUT_ERROR_TRACE("Wrong format string: No Token!\n");
            return NULL;
        }
        ++pAnchor;
        pQualityString = NSCharHelper::TrimLeftSpace(pAnchor, pEnd);
        if (!pQualityString) {
            OUTPUT_ERROR_TRACE("Wrong format string: No Quality Value!\n");
            return NULL;
        }
    }

    tokenIndex = CTokenManager::Instance()->GetTokenID(
        TOKEN, pString, pTokenEnd - pString);
    if (tokenIndex != INVALID_TOKEN_ID) {
        size_t weightLen = 0;
        if (pQualityString &&
            CWeightFieldValue::GetQualityByString(
                pQualityString, pEnd - pQualityString, weight, weightLen) != CWeightFieldValue::SPR_OK) {
            return NULL;
        }
        if (static_cast<int>(weightLen) != pEnd - pQualityString) {
            return NULL;
        }
        void* pMem = buffer.Malloc(sizeof(CTokenIndexWeightFieldValue<TOKEN>));
        if (pMem) {
            pValue = new (pMem)
                     CTokenIndexWeightFieldValue<TOKEN>(
                     static_cast<size_t>(tokenIndex), weight);
        }
    }
    return pValue;
}


// ABNF:
//  extension = token [ "=" ( token / quoted-string ) ]
//  parameter = token "=" ( token / quoted-string )
template <int ID_TOKEN, int VALUE_TOKEN, bool MANDATORY_VALUE>
class CTokenPairFieldValue : public IFieldValue
{
public:
    CTokenPairFieldValue(int tokenKey, int tokenValue = INVALID_TOKEN_ID) :
        m_Key(tokenKey),
        m_bTokenValue(true),
        m_TokenValue(tokenValue),
        m_pStringValue(NULL)
    {
        ASSERT(MANDATORY_VALUE && tokenValue != -1);
    }

    CTokenPairFieldValue(int tokenKey, const char* pStringValue = NULL) :
        m_Key(tokenKey),
        m_bTokenValue(false),
        m_TokenValue(INVALID_TOKEN_ID),
        m_pStringValue(pStringValue)
    {
        ASSERT(MANDATORY_VALUE && pStringValue != NULL);
    }

    ~CTokenPairFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    CTokenIndexFieldValue<ID_TOKEN> m_Key;
    const bool m_bTokenValue;
    CTokenIndexFieldValue<VALUE_TOKEN> m_TokenValue;
    const char* m_pStringValue;
};

template <int ID_TOKEN, int VALUE_TOKEN, bool MANDATORY_VALUE>
IFieldValue*
CTokenPairFieldValue<ID_TOKEN, VALUE_TOKEN, MANDATORY_VALUE>::
CreateInstance(const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    IFieldValue* pValue = NULL;
    int tokenKey = -1;
    int tokenValue = -1;
    char* pQuotedString = NULL;
    const char* pEnd = pString + len;
    const char* pKeyEnd = NSCharHelper::FindChar('=', pString, pEnd);

    if (!pKeyEnd) {
        if (MANDATORY_VALUE) {
            OUTPUT_ERROR_TRACE("No Value for Token Key\n");
            return NULL;
        }
        pKeyEnd = pEnd;
    } else {
        pKeyEnd = NSCharHelper::TrimRightSpace(pString, pKeyEnd);
        if (pKeyEnd == NULL) {
            OUTPUT_ERROR_TRACE("Malformat of Token string: %s\n", pString);
            return NULL;
        }
        len = pKeyEnd - pString;
    }

    tokenKey = CTokenManager::Instance()->GetTokenID(ID_TOKEN, pString, len);
    if (tokenKey == INVALID_TOKEN_ID) {
        OUTPUT_ERROR_TRACE("Unknown token ID: %s\n", pString);
        return NULL;
    }

    if (pKeyEnd != pEnd) {
        const char* pValueString = NSCharHelper::TrimLeftSpace(pKeyEnd + 1, pEnd);
        if (!pValueString) {    // Blank value.
            OUTPUT_ERROR_TRACE("Blank value\n");
            return NULL;
        }
        if (*pValueString != '\"') {  // Token Value
            tokenValue = CTokenManager::Instance()->GetTokenID(
                VALUE_TOKEN, pValueString, pEnd - pValueString);
            if (tokenValue == INVALID_TOKEN_ID) {
                OUTPUT_ERROR_TRACE("Unknown token ID, %s\n", pValueString);
                return NULL;
            }
        } else {
            if (*(pEnd - 1) != '\"') {
                OUTPUT_ERROR_TRACE("Wrong quoted string\n");
                return NULL;
            }
            size_t quotedLen = pEnd - pValueString;
            pQuotedString = reinterpret_cast<char*>(buffer.Malloc(quotedLen));
            if (!pQuotedString) {
                OUTPUT_ERROR_TRACE("Can not allocate memory from buffer\n");
                return NULL;
            }
            memcpy(pQuotedString, pValueString, quotedLen - 1);
            pQuotedString[quotedLen - 1] = '\0';
        }
    }

    void* pMem = buffer.Malloc(sizeof(CTokenPairFieldValue<ID_TOKEN, VALUE_TOKEN, MANDATORY_VALUE>));
    if (pMem) {
        pValue = pQuotedString ?
                 new (pMem) CTokenPairFieldValue<ID_TOKEN, VALUE_TOKEN, MANDATORY_VALUE>(tokenKey, pQuotedString) :
                 new (pMem) CTokenPairFieldValue<ID_TOKEN, VALUE_TOKEN, MANDATORY_VALUE>(tokenKey, tokenValue);
    }
    return pValue;
}

template <int ID_TOKEN, int VALUE_TOKEN, bool MANDATORY_VALUE>
bool CTokenPairFieldValue<ID_TOKEN, VALUE_TOKEN, MANDATORY_VALUE>::
Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    char* pEnd = pBuffer + len;
    bool bFinished = m_Key.Print(pBuffer, len, pOutPrintLen);
    if (!bFinished || (!m_bTokenValue && !m_pStringValue)) {
        return bFinished;
    }

    // Handle Value String
    char* pCur = pBuffer + *pOutPrintLen;
    if (pCur == pEnd) {
        return false;
    }
    *pCur++ = '=';
    ++*pOutPrintLen;
    if (pCur == pEnd) {
        return false;
    }

    size_t filled = 0;
    bFinished = false;
    if (m_bTokenValue) {
        bFinished = m_TokenValue.Print(pCur, pEnd - pCur, &filled);
    } else {
        filled = snprintf(pCur, pEnd - pCur, "\"%s\"", m_pStringValue);
        ASSERT(filled > 0);
        char ch = pCur[filled - 1];
        if (ch == '\0') {
            bFinished = true;
            --filled;
        } else if (filled > 1 && ch == '\"') {
            bFinished = true;
        }
    }
    *pOutPrintLen += filled;
    return bFinished;
}


// ABNF:
//    value     = token *( OWS ";" OWS parameter )
//    parameter = token "=" ( token / quoted-string )
template <int TOKEN, int PARAM_ID_TOKEN, int PARAM_VALUE_TOKEN>
class CTokenParamsFieldValue : public IFieldValue
{
public:
    typedef CTokenPairFieldValue<PARAM_ID_TOKEN, PARAM_VALUE_TOKEN, true> tParameter;

    CTokenParamsFieldValue(int token, CForwardList& paramList) :
        m_TokenIndexObject(token), m_ParamsList(paramList) {}
    CTokenParamsFieldValue(CTokenParamsFieldValue& value) :
        m_TokenIndexObject(value.m_TokenIndexObject),
        m_ParamsList(value.m_ParamsList) {}

    ~CTokenParamsFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    CTokenIndexFieldValue<TOKEN> m_TokenIndexObject;
    CForwardList m_ParamsList;

    DISALLOW_COPY_CONSTRUCTOR(CTokenParamsFieldValue);
    DISALLOW_DEFAULT_CONSTRUCTOR(CTokenParamsFieldValue);
    DISALLOW_ASSIGN_OPERATOR(CTokenParamsFieldValue);
};

template <int TOKEN, int PARAM_ID_TOKEN, int PARAM_VALUE_TOKEN>
bool CTokenParamsFieldValue<TOKEN, PARAM_ID_TOKEN, PARAM_VALUE_TOKEN>::
Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    char* pCur = pBuffer;
    char* pEnd = pBuffer + len;
    size_t totalPrintLen = 0;
    size_t curPrintLen = 0;
    bool bFinished = m_TokenIndexObject.Print(pCur, pEnd - pCur, &curPrintLen);
    pCur += curPrintLen;
    totalPrintLen += curPrintLen;
    if (!bFinished) {
        return false;
    }

    CForwardList::Iterator iter = m_ParamsList.Begin();
    CForwardList::Iterator iterEnd = m_ParamsList.End();
    while (iter != iterEnd) {
        bFinished = false;
        if (pCur >= pEnd) {
            break;
        }
        tParameter* pParam = reinterpret_cast<tParameter*>(m_ParamsList.DataAt(iter));
        ASSERT(pParam);

        *pCur++ = ';';
        ++totalPrintLen;
        if (pCur >= pEnd) {
            break;
        }
        bFinished = pParam->Print(pCur, pEnd - pCur, &curPrintLen);
        totalPrintLen += curPrintLen;
        if (!bFinished) {
            break;
        }
        ++iter;
    }
    *pOutPrintLen = totalPrintLen;
    return bFinished;
}

template <int TOKEN, int PARAM_ID_TOKEN, int PARAM_VALUE_TOKEN>
IFieldValue* CTokenParamsFieldValue<TOKEN, PARAM_ID_TOKEN, PARAM_VALUE_TOKEN>::
CreateInstance(const char* pString, size_t len, CLazyBuffer& buffer)
{
    ASSERT(len > 0);

    const char* pEnd = pString + len;
    const char* pTokenEnd = NULL;
    const char* pParam = NSCharHelper::FindChar(';', pString, pEnd);
    if (pParam == NULL) {
        pTokenEnd = pEnd;
    } else {
        pTokenEnd = NSCharHelper::TrimRightSpace(pString, pParam);
        if (pTokenEnd == NULL) {
            OUTPUT_ERROR_TRACE("Blank Token String\n");
            return NULL;
        }
        ++pParam;
    }

    int tokenIndex = CTokenManager::Instance()->GetTokenID(
        TOKEN, pString, pTokenEnd - pString);
    if (tokenIndex == INVALID_TOKEN_ID) {
        OUTPUT_ERROR_TRACE("Unknown Token String: %s\n", pString);
        return NULL;
    }

    void* pMem = buffer.Malloc(sizeof(CTokenParamsFieldValue<TOKEN, PARAM_ID_TOKEN, PARAM_VALUE_TOKEN>));
    if (!pMem) {
        OUTPUT_ERROR_TRACE("Allocate Memory failed\n");
        return NULL;
    }

    CForwardList paramList(&buffer);
    if (pTokenEnd == pEnd) {
        // Only Token
        return new (pMem) CTokenParamsFieldValue<TOKEN, PARAM_ID_TOKEN, PARAM_VALUE_TOKEN>(tokenIndex, paramList);
    }

    // Handle parameters.
    bool bRes = true;
    do {
        pParam = NSCharHelper::TrimLeftSpace(pParam, pEnd);
        if (!pParam) {
            OUTPUT_ERROR_TRACE("Blank parameter string\n");
            return NULL;
        }
        const char* pAnchor = NSCharHelper::FindChar(';', pParam, pEnd);
        const char* pParamBegin = pParam;
        const char* pParamEnd = NULL;
        if (pAnchor) {
            pParamEnd = NSCharHelper::TrimRightSpace(pParam, pAnchor);
            if (pParamEnd == NULL) {
                OUTPUT_ERROR_TRACE("Blank parameter string\n");
                return NULL;
            }
            pParam = pAnchor + 1;
        } else {
            pParamEnd = pEnd;
            pParam = NULL;
        }
        tParameter* pParamObject = reinterpret_cast<tParameter*>(
                CTokenPairFieldValue<PARAM_ID_TOKEN, PARAM_VALUE_TOKEN, true>::
                CreateInstance(pParamBegin, pParamEnd - pParamBegin, buffer));
        if (!paramList.PushBack(pParamObject)) {
            bRes = false;
            break;
        }        
    } while (pParam);

    return bRes ?
        new (pMem) CTokenParamsFieldValue<TOKEN, PARAM_ID_TOKEN, PARAM_VALUE_TOKEN>(tokenIndex, paramList) : NULL;
}

#endif
