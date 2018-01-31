/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpHeaderFieldDefs.h"
#include "HTTPBase/FieldValue.h"
#include "HttpToken.h"
#include "HttpFieldValue.h"
#include "Common/Macros.h"
#include "Common/CharHelper.h"

#define CHAR_NULL  0x00
#define CHAR_SPACE 0x20 // ' '
#define CHAR_COMMA 0x2C // ','
#define CHAR_SEMICOLON 0x3B // ';'

///////////////////////////////////////////////////////////////////////////////
//
// CHttpHeaderFieldDefs Implemenation
//
///////////////////////////////////////////////////////////////////////////////

// Make sure the initialization take place when loading
static const CHeaderField::GlobalConfig*
s_pRequestGlobalConfig = CHttpHeaderFieldDefs::GetRequestGlobalConfig();
static const CHeaderField::GlobalConfig*
s_pResponseGlobalConfig = CHttpHeaderFieldDefs::GetResponseGlobalConfig();

const CHeaderField::GlobalConfig* CHttpHeaderFieldDefs::GetRequestGlobalConfig()
{
    static const tStringIDMap::value_type s_ReqMapInitValues[] = {
        tStringIDMap::value_type("Host",                REQ_FN_HOST),
        tStringIDMap::value_type("User-Agent",          REQ_FN_USER_AGENT),
        tStringIDMap::value_type("Accept",              REQ_FN_ACCEPT),
        tStringIDMap::value_type("Accept-Charset",      REQ_FN_ACCEPT_CHARSET),
        tStringIDMap::value_type("Accept-Encoding",     REQ_FN_ACCEPT_ENCODING),
        tStringIDMap::value_type("Accept-Language",     REQ_FN_ACCEPT_LANGUAGE),
        tStringIDMap::value_type("Authorization",       REQ_FN_AUTHORIZATION),
        tStringIDMap::value_type("Cache-Control",       REQ_FN_CACHE_CONTROL),
        tStringIDMap::value_type("Connection",          REQ_FN_CONNECTION),
        tStringIDMap::value_type("Content-Encoding",    REQ_FN_CONTENT_ENCODING),
        tStringIDMap::value_type("Content-Language",    REQ_FN_CONTENT_LANGUAGE),
        tStringIDMap::value_type("Content-Length",      REQ_FN_CONTENT_LENGTH),
        tStringIDMap::value_type("Content-MD5",         REQ_FN_CONTENT_MD5),
        tStringIDMap::value_type("Content-Range",       REQ_FN_CONTENT_RANGE),
        tStringIDMap::value_type("Content-Type",        REQ_FN_CONTENT_TYPE),
        tStringIDMap::value_type("Cookie",              REQ_FN_COOKIE),
        tStringIDMap::value_type("Expect",              REQ_FN_EXPECT),
        tStringIDMap::value_type("From",                REQ_FN_FROM),
        tStringIDMap::value_type("If-Match",            REQ_FN_IF_MATCH),
        tStringIDMap::value_type("If-Modified-Since",   REQ_FN_IF_MODIFIED_SINCE),
        tStringIDMap::value_type("If-None-Match",       REQ_FN_IF_NONE_MATCH),
        tStringIDMap::value_type("If-Range",            REQ_FN_IF_RANGE),
        tStringIDMap::value_type("If-Unmodified-Since", REQ_FN_IF_UNMODIFIED_SINCE),
        tStringIDMap::value_type("Last-Modified",       REQ_FN_LAST_MODIFIED),
        tStringIDMap::value_type("Max-Forwards",        REQ_FN_MAX_FORWARDS),
        tStringIDMap::value_type("Pragma",              REQ_FN_PRAGMA),
        tStringIDMap::value_type("Proxy-Authorization", REQ_FN_PROXY_AUTHORIZATION),
        tStringIDMap::value_type("Range",               REQ_FN_RANGE),
        tStringIDMap::value_type("Referer",             REQ_FN_REFERER),
        tStringIDMap::value_type("TE",                  REQ_FN_TE),
        tStringIDMap::value_type("Trailer",             REQ_FN_TRAILER),
        tStringIDMap::value_type("Transfer-Encoding",   REQ_FN_TRANSFER_ENCODING),
    };

    static const tStringIDMap s_ReqFieldNameMap(
        s_ReqMapInitValues,
        s_ReqMapInitValues + COUNT_OF_ARRAY(s_ReqMapInitValues),
        NSCharHelper::StringCaseCompare);

    static const CHeaderField::ItemConfig s_ReqItemsConfig[] = {
        // pItemName, BitSet, DelimitedChar, CreatorFunction
        { "Host", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        {
            "User-Agent",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_SPACE,
            CProductFieldValue::CreateInstance
        },
        {
            "Accept",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CHttpAcceptFieldValue::CreateInstance
        },
        {
            "Accept-Charset",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexWeightFieldValue<CHttpTokenMap::CATEGORY_CHARSET>::CreateInstance
        },
        {
            "Accept-Encoding",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexWeightFieldValue<CHttpTokenMap::CATEGORY_ENCODING>::CreateInstance
        },
        {
            "Accept-Language",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CHttpAcceptLangFieldValue::CreateInstance
        },
        { "Authorization", 0, CHAR_NULL, NULL },
        { "Cache-Control", 0, CHAR_NULL, NULL },
        {
            "Connection",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_CONNECTION>::CreateInstance
        },
        {
            "Content-Encoding",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>::CreateInstance
        },
        {
            "Content-Language",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CStringFieldValue::CreateInstance
        },
        { "Content-Length", 0, CHAR_NULL, CIntFieldValue::CreateInstance },
        { "Content-MD5", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "Content-Range", 0, CHAR_NULL, NULL },     // REQ_FN_CONTENT_RANGE ??
        {
            "Content-Type",
            0,
            CHAR_NULL,
            CTokenParamsFieldValue<
                CHttpTokenMap::CATEGORY_MEDIA_TYPE,
                CHttpTokenMap::CATEGORY_MEDIA_TYPE_PARAM_ID,
                CHttpTokenMap::CATEGORY_CHARSET>::CreateInstance
        },
        {
            "Cookie",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_SEMICOLON,
            NULL
        },
        { "Expect", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "From", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "If-Match", 0, CHAR_NULL, NULL },
        { "If-Modified-Since", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "If-None-Match", 0, CHAR_NULL, NULL },
        { "If-Range", 0, CHAR_NULL, NULL },
        { "If-Unmodified-Since", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "Last-Modified", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "Max-Forwards", 0, CHAR_NULL, CIntFieldValue::CreateInstance },
        { "Pragma", CHeaderField::ItemConfig::MULTI_VALUES_FLAG, CHAR_COMMA, NULL },
        { "Proxy-Authorization", 0, CHAR_NULL, NULL},
        { "Range", 0, CHAR_NULL, NULL },
        { "Referer", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        {
            "TE",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            NULL
        },
        {
            "Trailer",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            NULL
        },
        { "Transfer-Encoding", 0, CHAR_COMMA, CHttpTransferEncodingFieldValue::CreateInstance },
    };

    static const CHeaderField::GlobalConfig s_ReqGlobalConfig = {
        true,
        COUNT_OF_ARRAY(s_ReqItemsConfig),
        s_ReqItemsConfig,
        &s_ReqFieldNameMap
    };

    return &s_ReqGlobalConfig;
}

const CHeaderField::GlobalConfig* CHttpHeaderFieldDefs::GetResponseGlobalConfig()
{
    static const tStringIDMap::value_type s_RespMapInitValues[] = {
        tStringIDMap::value_type("Age",                 RESP_FN_AGE),
        tStringIDMap::value_type("Allow",               RESP_FN_ALLOW),
        tStringIDMap::value_type("Authorization",       RESP_FN_AUTHORIZATION),
        tStringIDMap::value_type("Cache-Control",       RESP_FN_CACHE_CONTROL),
        tStringIDMap::value_type("Connection",          RESP_FN_CONNECTION),
        tStringIDMap::value_type("Content-Encoding",    RESP_FN_CONTENT_ENCODING),
        tStringIDMap::value_type("Content-Language",    RESP_FN_CONTENT_LANGUAGE),
        tStringIDMap::value_type("Content-Length",      RESP_FN_CONTENT_LENGTH),
        tStringIDMap::value_type("Content-Location",    RESP_FN_CONTENT_LOCATION),
        tStringIDMap::value_type("Content-MD5",         RESP_FN_CONTENT_MD5),
        tStringIDMap::value_type("Content-Range",       RESP_FN_CONTENT_RANGE),
        tStringIDMap::value_type("Content-Type",        RESP_FN_CONTENT_TYPE),
        tStringIDMap::value_type("Date",                RESP_FN_DATE),
        tStringIDMap::value_type("ETag",                RESP_FN_ETAG),
        tStringIDMap::value_type("Expires",             RESP_FN_EXPIRES),
        tStringIDMap::value_type("Last-Modified",       RESP_FN_LAST_MODIFIED),
        tStringIDMap::value_type("Location",            RESP_FN_LOCATION),
        tStringIDMap::value_type("Max-Forwards",        RESP_FN_MAX_FORWARDS),
        tStringIDMap::value_type("Pragma",              RESP_FN_PRAGMA),
        tStringIDMap::value_type("Proxy-Authenticate",  RESP_FN_PROXY_AUTHENTICATE),
        tStringIDMap::value_type("Range",               RESP_FN_RANGE),
        tStringIDMap::value_type("Referer",             RESP_FN_REFERER),
        tStringIDMap::value_type("Retry-After",         RESP_FN_RETRY_AFTER),
        tStringIDMap::value_type("Server",              RESP_FN_SERVER),
        tStringIDMap::value_type("Set-Cookie",          RESP_FN_SET_COOKIE),
        tStringIDMap::value_type("Set-Cookie2",         RESP_FN_SET_COOKIE2),
        tStringIDMap::value_type("Trailer",             RESP_FN_TRAILER),
        tStringIDMap::value_type("Transfer-Encoding",   RESP_FN_TRANSFER_ENCODING),
        tStringIDMap::value_type("Upgrade",             RESP_FN_UPGRADE),
        tStringIDMap::value_type("Vary",                RESP_FN_VARY),
        tStringIDMap::value_type("Via",                 RESP_FN_VIA),
        tStringIDMap::value_type("Warning",             RESP_FN_WARNING),
        tStringIDMap::value_type("WWW-Authenticate",    RESP_FN_WWW_AUTHENTICATE)
    };

    static const tStringIDMap s_RespFieldNameMap(
        s_RespMapInitValues,
        s_RespMapInitValues + COUNT_OF_ARRAY(s_RespMapInitValues),
        NSCharHelper::StringCaseCompare);

    static const CHeaderField::ItemConfig s_RespItemsConfig[] = {
        { "Age", 0, CHAR_NULL, CIntFieldValue::CreateInstance },
        { "Allow", 0, CHAR_NULL, NULL              },     // RESP_FN_ALLOW, HTTP Response Field (for 405)
        { "Authorization", 0, CHAR_NULL, NULL       },     // RESP_FN_AUTHORIZATION, ??
        { "Cache-Control", 0, CHAR_NULL, NULL      },     // RESP_FN_CACHE_CONTROL
        {
            "Connection",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_CONNECTION>::CreateInstance
        },
        {
            "Content-Encoding",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>::CreateInstance
        },
        {
            "Content-Language",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CStringFieldValue::CreateInstance
        },
        { "Content-Length", 0, CHAR_NULL, CIntFieldValue::CreateInstance },
        { "Content-Location", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "Content-MD5", 0, CHAR_NULL, NULL        },     // RESP_FN_CONTENT_MD5
        { "Content-Range", 0, CHAR_NULL, NULL      },     // RESP_FN_CONTENT_RANGE
        {
            "Content-Type",
            0,
            CHAR_NULL,
            CTokenParamsFieldValue<
                CHttpTokenMap::CATEGORY_MEDIA_TYPE,
                CHttpTokenMap::CATEGORY_MEDIA_TYPE_PARAM_ID,
                CHttpTokenMap::CATEGORY_CHARSET>::CreateInstance
        },
        { "Date", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "ETag", 0, CHAR_NULL, NULL               },     // RESP_FN_ETAG, HTTP Response Field
        { "Expires", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "Last-Modified", 0, CHAR_NULL, CDateFieldValue::CreateInstance },
        { "Location", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "Max-Forwards", 0, CHAR_NULL, CIntFieldValue::CreateInstance },
        { "Pragma", 0, CHAR_NULL, NULL             },     // RESP_FN_PRAGMA
        { "Proxy-Authenticate", 0, CHAR_NULL, NULL },     // RESP_FN_PROXY_AUTHENTICATE, HTTP Response Field(for 407)
        { "Range", 0, CHAR_NULL, NULL              },     // RESP_FN_RANGE
        { "Referer", 0, CHAR_NULL, CStringFieldValue::CreateInstance },
        { "Retry-After", 0, CHAR_NULL, NULL        },     // RESP_FN_RETRY_AFTER, HTTP Response Field(for 503)
        {
            "Server",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_SPACE,
            CProductFieldValue::CreateInstance
        },
        {
            "Set-Cookie",
            CHeaderField::ItemConfig::MULTI_ITEMS_FLAG,
            CHAR_NULL,
            CHttpSetCookieFieldValue::CreateInstance
        },
        {
            "Set-Cookie2",
            CHeaderField::ItemConfig::MULTI_ITEMS_FLAG,
            CHAR_NULL,
            CHttpSetCookieFieldValue::CreateInstance
        },
        { "Trailer", 0, CHAR_NULL, NULL            },     // RESP_FN_TRAILER
        {
            "Transfer-Encoding",
            CHeaderField::ItemConfig::MULTI_VALUES_FLAG,
            CHAR_COMMA,
            CHttpTransferEncodingFieldValue::CreateInstance
        },
        { "Upgrade", 0, CHAR_NULL, NULL            },     // RESP_FN_UPGRADE
        { "Vary", 0, CHAR_NULL, NULL               },     // RESP_FN_VARY, HTTP Response Field
        { "Via", 0, CHAR_NULL, NULL                },     // RESP_FN_VIA
        { "Warning", 0, CHAR_NULL, NULL            },     // RESP_FN_WARNING
        { "WWW-Authenticate", 0, CHAR_NULL, NULL   },     // RESP_FN_WWW_AUTHENTICATE, HTTP Response Field(for 401)
    };

    static const CHeaderField::GlobalConfig s_RespGlobalConfig = {
        true,
        COUNT_OF_ARRAY(s_RespItemsConfig),
        s_RespItemsConfig,
        &s_RespFieldNameMap
    };

    return &s_RespGlobalConfig;
}

const CHeaderField::GlobalConfig* CHttpHeaderFieldDefs::GetConnectRequestGlobalConfig()
{
    // Connect Request only have Host and User Agent head field.
    static CHeaderField::GlobalConfig s_ConnectRequestGlobalConfig(
        GetRequestGlobalConfig()->bSupportExtension,
        2,
        GetRequestGlobalConfig()->pItems,
        GetRequestGlobalConfig()->pNamesMap);
    return &s_ConnectRequestGlobalConfig;
}

