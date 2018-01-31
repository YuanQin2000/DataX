/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_FIELD_VALUE_H__
#define __HTTP_FIELD_VALUE_H__

#include <sys/time.h>
#include <cstring>
#include "Common/ForwardList.h"
#include "HTTPBase/FieldValue.h"
#include "HTTPBase/Token.h"
#include "HttpToken.h"
#include "HttpTokenDefs.h"
#include "HttpCookieData.h"

using std::memcpy;

///////////////////////////////////////////////////////////////////////////////
//
// CHttpAcceptFieldValue Definition
//
//
// ABNF:
// Accept = #( media-range [ accept-params ] )
//    media-range = ( "*/*"
//                / ( type "/" "*" )
//                / ( type "/" subtype )
//                ) *( OWS ";" OWS parameter )
//    accept-params = weight *( accept-ext )
//    accept-ext = OWS ";" OWS token [ "=" ( token / quoted-string ) ]
///////////////////////////////////////////////////////////////////////////////
class CHttpAcceptFieldValue : public IFieldValue
{
public:
    typedef CTokenParamsFieldValue<
        CHttpTokenMap::CATEGORY_MEDIA_TYPE,
        CHttpTokenMap::CATEGORY_MEDIA_TYPE_PARAM_ID,
        CHttpTokenMap::CATEGORY_CHARSET> tMediaRange;
    typedef CTokenPairFieldValue<
        CHttpTokenMap::CATEGORY_ACCEPT_EXT_ID,
        CHttpTokenMap::CATEGORY_ACCEPT_EXT_VALUE,
        false> tAcceptExtension;

    CHttpAcceptFieldValue(
        tMediaRange& mediaRange,
        CForwardList& extList,
        unsigned short weight = 0) :
        m_MediaRange(mediaRange),
        m_Weight(weight),
        m_ExtensionList(extList) {}
    ~CHttpAcceptFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    tMediaRange m_MediaRange;
    CWeightFieldValue m_Weight;
    CForwardList m_ExtensionList;

    DISALLOW_DEFAULT_CONSTRUCTOR(CHttpAcceptFieldValue);
    DISALLOW_COPY_CONSTRUCTOR(CHttpAcceptFieldValue);
    DISALLOW_ASSIGN_OPERATOR(CHttpAcceptFieldValue);
};


///////////////////////////////////////////////////////////////////////////////
//
// CHttpTransferEncodingFieldValue Definition
//
//
// ABNF:
// Transfer-Encoding = 1#transfer-coding
//    transfer-coding = "chunked" ; Section 4.1
//                    / "compress" ; Section 4.2.1
//                    / "deflate" ; Section 4.2.2
//                    / "gzip" ; Section 4.2.3
//                    / transfer-extension
//    transfer-extension = token *( OWS ";" OWS transfer-parameter )
///////////////////////////////////////////////////////////////////////////////
class CHttpTransferEncodingFieldValue : public IFieldValue
{
public:
    typedef CTokenParamsFieldValue<
        CHttpTokenMap::CATEGORY_TRANSFER_EXT,
        CHttpTokenMap::CATEGORY_TRANSFER_EXT_PARAM_ID,
        CHttpTokenMap::CATEGORY_TRANSFER_EXT_PARAM_VALUE> tExtension;

    enum ValueType {
        VT_CHUNKED,
        VT_ENCODING_TOKEN,
        VT_EXTENSION
    };

    CHttpTransferEncodingFieldValue() :
        m_ValueType(VT_CHUNKED),
        m_Value() {}
    CHttpTransferEncodingFieldValue(tTokenID encoding) :
        m_ValueType(VT_ENCODING_TOKEN),
        m_Value(encoding) {}
    CHttpTransferEncodingFieldValue(tExtension& extObject) :
        m_ValueType(VT_EXTENSION),
        m_Value(extObject) {}
    ~CHttpTransferEncodingFieldValue() {}

    ValueType GetValueType() const { return m_ValueType; }
    tTokenID GetEncodingType() const
    {
        ASSERT(m_ValueType == VT_ENCODING_TOKEN);
        return m_Value.Encoding.GetToken();
    }
    const tExtension& GetExtension() const
    {
        ASSERT(m_ValueType == VT_EXTENSION);
        return m_Value.ExtensionObject;
    }

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    ValueType m_ValueType;
    union TEValue {
        CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING> Encoding;
        tExtension ExtensionObject;

        TEValue() {}
        TEValue(tTokenID encoding) : Encoding(encoding) {}
        TEValue(tExtension& extObject) : ExtensionObject(extObject) {}
        ~TEValue() {}
    } m_Value;

    static const char s_chunkedLabel[];
};


///////////////////////////////////////////////////////////////////////////////
//
// CHttpRetryAfterFieldValue Definition
//
///////////////////////////////////////////////////////////////////////////////
class CHttpRetryAfterFieldValue : public IFieldValue
{
public:
    CHttpRetryAfterFieldValue(tSecondTick tv) :
        m_bIsDate(true),
        m_Value(tv) {}

    CHttpRetryAfterFieldValue(int seconds) :
        m_bIsDate(false),
        m_Value(seconds) {}

    ~CHttpRetryAfterFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    bool m_bIsDate;
    union RFValue {
        RFValue(tSecondTick tv) : Date(tv) {}
        RFValue(int sec) : DelaySeconds(sec) {}
        ~RFValue() {}

        CDateFieldValue Date;
        int DelaySeconds;
    } m_Value;
};


///////////////////////////////////////////////////////////////////////////////
//
// CHttpAcceptLangFieldValue Definition
//
///////////////////////////////////////////////////////////////////////////////
class CHttpAcceptLangFieldValue : public IFieldValue
{
public:
    CHttpAcceptLangFieldValue(const char* pLang, unsigned short weight) :
        m_pLangName(pLang),
        m_Weight(weight) {}

    ~CHttpAcceptLangFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    const char* m_pLangName;
    CWeightFieldValue m_Weight;
};


///////////////////////////////////////////////////////////////////////////////
//
// CHttpSetCookieFieldValue Definition
//
///////////////////////////////////////////////////////////////////////////////
class CHttpSetCookieFieldValue : public IFieldValue
{
public:
    CHttpSetCookieFieldValue(
        char* pDomain,
        const char* pP,
        const char* pN,
        const char* pV,
        tSecondTick expire = 0,
        bool bSec = false,
        bool bWild = false) :
        m_CookieValue(pDomain, pP, pN, pV, expire, bSec, bWild) {}
    ~CHttpSetCookieFieldValue() {}

    CHttpCookieData* GetData() { return &m_CookieValue; }
    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    DECLARE_STATIC_CREATE_FUNCTION;

private:
    union AttributeValue {
        tSecondTick ExpiredTime;
        char* pString;
    };

    static bool CreateNameValue(
        const char* pString,
        size_t len,
        CLazyBuffer& buffer,
        char** pOutName, char** pOutValue);

    static CookieAttrID ParseAttribute(
        const char* pString,
        size_t len,
        CLazyBuffer& buffer,
        AttributeValue* pOutValue);

private:
    CHttpCookieData m_CookieValue;
};


///////////////////////////////////////////////////////////////////////////////
//
// CHttpCookieFieldValue Definition
//
///////////////////////////////////////////////////////////////////////////////
class CHttpCookieFieldValue : public IFieldValue
{
public:
    CHttpCookieFieldValue(HttpCookieValue* pValue) :
        m_pValue(pValue) { ASSERT(pValue); }
    ~CHttpCookieFieldValue() {}

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    // Disable the DECLARE_STATIC_CREATE_FUNCTION
    // since we won't create cookie from string.

private:
    HttpCookieValue* m_pValue;  // Not owned
};

#endif