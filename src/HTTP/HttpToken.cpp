/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpToken.h"
#include "Tracker/Trace.h"
#include "Common/Macros.h"
#include "HttpTokenDefs.h"
#include "HTTPBase/Token.h"

const char* CHttpTokenMap::s_HttpMethod[] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "TRACE",
    "OPTIONS"
};

const char* CHttpTokenMap::s_HttpVersion[] = {
    "HTTP/1.0",
    "HTTP/1.1",
    "HTTP/2.0",
};

// CATEGORY_MEDIA_TYPE
const char* CHttpTokenMap::s_MediaTypes[] = {
    "text/html",             // MEDIA_TEXT_HTML
    "text/plain",            // MEDIA_TEXT_PLAIN
    "application/xhtml+xml", // MEDIA_APP_XHTML_XML
    "application/xml",       // MEDIA_APP_XML
    "*/*",                   // MEDIA_ALL
};

// CATEGORY_ENCODING
const char* CHttpTokenMap::s_Encodings[] = {
    "compress",   // ENCODING_COMPRESS
    "deflate",    // ENCODING_DEFLAT
    "gzip",       // ENCODING_GZIP
    "x-gzip",     // ENCODING_GZIP_X
    "x-compress", // ENCODING_COMPRESS_X
};

// CATEGORY_CHARSET
const char* CHttpTokenMap::s_Charsets[] = {
    "unicode-1-1", // CHARSET_UNICODE
    "utf-8",       // CHARSET_UTF8
    "iso-8859-5",  // CHARSET_ISO8856
    "gb2312",      // CHARSET_GB2312
};

// CATEGORY_CONNECTION
const char* CHttpTokenMap::s_Connections[] = {
    "close",        // CO_CLOSE
    "keep-alive",   // CO_KEEP_ALIVE
    "upgrade",      // CO_UPGRADE
};

// CATEGORY_MEDIA_TYPE_PARAM_ID
const char* CHttpTokenMap::s_MediaTypeParamToken[] = {
    "charset",
};

// CATEGORY_ACCEPT_EXT_ID
const char* CHttpTokenMap::s_AcceptExtToken[] = {
    // TODO
    "unknown",
};

// CATEGORY_ACCEPT_EXT_VALUE
const char* CHttpTokenMap::s_AcceptExtTokenValue[] = {
    // TODO
    "unknown",
};

// CATEGORY_TRANSFER_EXT
const char* CHttpTokenMap::s_TransferEncodingExtToken[] = {
    // TODO
    "unknown",
};

// CATEGORY_TRANSFER_EXT_PARAM_ID
const char* CHttpTokenMap::s_TransferEncodingExtParamToken[] = {
    // TODO
    "unknown",
};

// CATEGORY_TRANSFER_EXT_PARAM_VALUE
const char* CHttpTokenMap::s_TransferEncodingExtParamTokenValue[] = {
    // TODO
    "unknown",
};

// CATEGORY_COOKIE_ATTR_ID
const char* CHttpTokenMap::s_CookieAttrName[] = {
    "domain",     // COOKIE_DOMAIN,
    "path",       // COOKIE_PATH,
    "secure",     // COOKIE_SECURE,
    "expires",    // COOKIE_EXPIRES,
    "max-age",    // COOKIE_MAX_AGE,
    "comment",    // COOKIE_COMMENT,
    "commenturi", // COOKIE_COMMENT_URI,
    "version",    // COOKIE_VERSION,
    "discard",    // COOKIE_DISCARD,
    "port",       // COOKIE_PORT,
};


const CHttpTokenMap::StringArrayEntry CHttpTokenMap::s_StringArrays[] = {
    { COUNT_OF_ARRAY(s_HttpMethod), s_HttpMethod },
    { COUNT_OF_ARRAY(s_HttpVersion), s_HttpVersion },
    { COUNT_OF_ARRAY(s_MediaTypes), s_MediaTypes },
    { COUNT_OF_ARRAY(s_Encodings), s_Encodings },
    { COUNT_OF_ARRAY(s_Charsets), s_Charsets },
    { COUNT_OF_ARRAY(s_Connections), s_Connections },
    { COUNT_OF_ARRAY(s_MediaTypeParamToken), s_MediaTypeParamToken },
    { COUNT_OF_ARRAY(s_AcceptExtToken), s_AcceptExtToken },
    { COUNT_OF_ARRAY(s_AcceptExtTokenValue), s_AcceptExtTokenValue },
    { COUNT_OF_ARRAY(s_TransferEncodingExtToken), s_TransferEncodingExtToken },
    { COUNT_OF_ARRAY(s_TransferEncodingExtParamToken), s_TransferEncodingExtParamToken },
    { COUNT_OF_ARRAY(s_TransferEncodingExtParamTokenValue), s_TransferEncodingExtParamTokenValue },
    { COUNT_OF_ARRAY(s_CookieAttrName), s_CookieAttrName }
};


CHttpTokenMap::CHttpTokenMap()
{
    memset(m_TokenEntry, 0, sizeof(m_TokenEntry));

    for (size_t i = 0; i < sizeof(m_TokenEntry) / sizeof(m_TokenEntry[0]); ++i) {
        m_TokenEntry[i] = CStringIndex::CreateInstance(
            s_StringArrays[i].Count, s_StringArrays[i].pArray, false);
        if (m_TokenEntry[i] == NULL) {
            ASSERT(false, "Create Instance failed from NO.%d\n", i);
            break;
        }
    }
}

CHttpTokenMap::~CHttpTokenMap()
{
}

const char* CHttpTokenMap::GetTokenString(int categoryID, int tokenID) const
{
    ASSERT(categoryID >= 0 && categoryID < CATEGORY_COUNT);

    CStringIndex* pTokenMap = m_TokenEntry[categoryID];
    ASSERT(pTokenMap);
    return pTokenMap->GetStringByIndex(tokenID);
}

int CHttpTokenMap::GetTokenID(
    int categoryID, const char* pString, size_t len) const
{
    ASSERT(categoryID >= 0 && categoryID < CATEGORY_COUNT);

    CStringIndex* pTokenMap = m_TokenEntry[categoryID];
    ASSERT(pTokenMap);
    return pTokenMap->GetIndexByString(pString, len);
}

bool CHttpTokenMap::CheckValidity(tTokenCategory categoryID, tTokenID tokenID) const
{
    return categoryID >= 0 &&
           categoryID < CATEGORY_COUNT &&
           tokenID >= 0 &&
           static_cast<size_t>(tokenID) < s_StringArrays[categoryID].Count;
}

static CTokenMapRegister g_HttpStdTokenMapReg(CHttpTokenMap::Instance());
