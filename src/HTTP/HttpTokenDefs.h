/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_TOKEN_DEFS_H__
#define __HTTP_TOKEN_DEFS_H__

#include "Common/Typedefs.h"

///////////////////////////////////////////////////////////////////////////////
//
// Http Token ID Defines
//
///////////////////////////////////////////////////////////////////////////////

enum HttpVersion {
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2_0,
    HTTP_VERSION_COUNT
};

enum RequestMethodID {
    REQUEST_METHOD_GET,         // Transfer a current representation of the target resource.
    REQUEST_METHOD_HEAD,        // Same as GET, but only transfer the status line and header section.
    REQUEST_METHOD_POST,        // Perform resource-specific processing on the request payload.
    REQUEST_METHOD_PUT,         // Replace all current representations of the target resource with the request payload.
    REQUEST_METHOD_DELETE,      // Remove all current representations of the target resource.
    REQUEST_METHOD_CONNECT,     // Establish a tunnel to the server identified by the target resource.
    REQUEST_METHOD_TRACE,       // Perform a message loop-back test along the path to the target resource.
    REQUEST_METHOD_OPTIONS,     // Describe the communication options for the target resource.
    REQUEST_METHOD_COUNT
};

enum MediaType {
    MEDIA_INVALID = -1,
    MEDIA_TEXT_HTML,
    MEDIA_TEXT_PLAIN,
    MEDIA_APP_XHTML_XML,
    MEDIA_APP_XML,
    MEDIA_ALL,
    MEDIA_COUNT
};

enum EncodingType {
    ENCODING_INVALID = -2,
    ENCODING_NONE = -1,
    ENCODING_COMPRESS,      // compress coding, LZW
    ENCODING_DEFLAT,        // deflat coding, LZ77
    ENCODING_GZIP,          // GZip coding, LZ77 & CRC
    ENCODING_XCOMPRESS,     // x-compress, Alias of compress
    ENCODING_XGZIP,         // x-gzip, Alias of gzip
    ENCODING_COUNT
};

// RFC 2978
enum CharsetID {
    CHARSET_INVALID = -1,
    CHARSET_UNICODE,
    CHARSET_UTF8,
    CHARSET_ISO8856,
    CHARSET_GB2312,
    CHARSET_COUNT
};

// RFC5646
// TODO: Complete
enum LanguageID {
    LANGID_INVALID = -2,
    LANGID_MIN = -1,
    LANGID_EN,
    LANGID_CN_SIMPLIFIED,
    LANGID_CN_TRADITIONAL,
    LANGID_COUNT
};

enum ConnectionOption {
    CO_CLOSE,
    CO_KEEP_ALIVE,
    CO_UPGRADE,
};

enum CookieAttrID {
    COOKIE_INVALID = -1,
    COOKIE_DOMAIN,
    COOKIE_PATH,
    COOKIE_SECURE,
    COOKIE_EXPIRES,
    COOKIE_MAX_AGE,
    COOKIE_COMMENT,
    COOKIE_COMMENT_URI,
    COOKIE_VERSION,
    COOKIE_DISCARD,
    COOKIE_PORT,
};

#endif