/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include "Compress/Compressor.h"
#include "HttpTokenDefs.h"

class CHeaderField;
class CAuthority;
class CUri;
class CLazyBuffer;

namespace NSHttpUtils
{

CHeaderField* CreateDefaultRequestHeaderField(CLazyBuffer& buffer, bool bFillPref = true);
bool SetHostField(CAuthority* pHost, CHeaderField* pHeaderField);
void SetCookieField(CUri* pUri, CHeaderField* pHeaderField);

CompressType Encoding2CompressType(EncodingType type);
EncodingType Compress2EncodingType(CompressType ct);

inline bool IsResponseOK(int statusCode)
{
    return statusCode >= 200 && statusCode < 300;
}

inline bool IsIdempotentMethod(RequestMethodID reqID)
{
    return reqID == REQUEST_METHOD_GET ||
           reqID == REQUEST_METHOD_HEAD ||
           reqID == REQUEST_METHOD_PUT ||
           reqID == REQUEST_METHOD_DELETE ||
           reqID == REQUEST_METHOD_OPTIONS;
}

};

#endif