/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_HEADER_FIELD_DEFS_H__
#define __HTTP_HEADER_FIELD_DEFS_H__

#include "HTTPBase/HeaderField.h"

class CHttpHeaderFieldDefs
{
public:
    enum RequestFieldName {
        REQ_FN_INVALID = -1,
        REQ_FN_MIN = -1,
        REQ_FN_HOST,
        REQ_FN_USER_AGENT,
        REQ_FN_ACCEPT,
        REQ_FN_ACCEPT_CHARSET,
        REQ_FN_ACCEPT_ENCODING,
        REQ_FN_ACCEPT_LANGUAGE,
        REQ_FN_AUTHORIZATION,
        REQ_FN_CACHE_CONTROL,
        REQ_FN_CONNECTION,
        REQ_FN_CONTENT_ENCODING,
        REQ_FN_CONTENT_LANGUAGE,
        REQ_FN_CONTENT_LENGTH,
        REQ_FN_CONTENT_MD5,
        REQ_FN_CONTENT_RANGE,
        REQ_FN_CONTENT_TYPE,
        REQ_FN_COOKIE,
        REQ_FN_EXPECT,
        REQ_FN_FROM,
        REQ_FN_IF_MATCH,
        REQ_FN_IF_MODIFIED_SINCE,
        REQ_FN_IF_NONE_MATCH,
        REQ_FN_IF_RANGE,
        REQ_FN_IF_UNMODIFIED_SINCE,
        REQ_FN_LAST_MODIFIED,
        REQ_FN_MAX_FORWARDS,
        REQ_FN_PRAGMA,
        REQ_FN_PROXY_AUTHORIZATION,
        REQ_FN_RANGE,
        REQ_FN_REFERER,
        REQ_FN_TE,
        REQ_FN_TRAILER,
        REQ_FN_TRANSFER_ENCODING,
        REQ_FN_MAX
    };

    enum RespFieldName {
        RESP_FN_INVALID = -1,
        RESP_FN_MIN = -1,
        RESP_FN_AGE,
        RESP_FN_ALLOW,
        RESP_FN_AUTHORIZATION,
        RESP_FN_CACHE_CONTROL,
        RESP_FN_CONNECTION,
        RESP_FN_CONTENT_ENCODING,
        RESP_FN_CONTENT_LANGUAGE,
        RESP_FN_CONTENT_LENGTH,
        RESP_FN_CONTENT_LOCATION,
        RESP_FN_CONTENT_MD5,
        RESP_FN_CONTENT_RANGE,
        RESP_FN_CONTENT_TYPE,
        RESP_FN_DATE,
        RESP_FN_ETAG,
        RESP_FN_EXPIRES,
        RESP_FN_LAST_MODIFIED,
        RESP_FN_LOCATION,
        RESP_FN_MAX_FORWARDS,
        RESP_FN_PRAGMA,
        RESP_FN_PROXY_AUTHENTICATE,
        RESP_FN_RANGE,
        RESP_FN_REFERER,
        RESP_FN_RETRY_AFTER,
        RESP_FN_SERVER,
        RESP_FN_SET_COOKIE,
        RESP_FN_SET_COOKIE2,
        RESP_FN_TRAILER,
        RESP_FN_TRANSFER_ENCODING,
        RESP_FN_UPGRADE,
        RESP_FN_VARY,
        RESP_FN_VIA,
        RESP_FN_WARNING,
        RESP_FN_WWW_AUTHENTICATE,
        RESP_FN_MAX
    };

    static const CHeaderField::GlobalConfig* GetRequestGlobalConfig();
    static const CHeaderField::GlobalConfig* GetResponseGlobalConfig();

    static const CHeaderField::GlobalConfig* GetConnectRequestGlobalConfig();
};

#endif