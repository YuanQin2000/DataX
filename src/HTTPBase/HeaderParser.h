/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_HEADER_PARSER_H__
#define __HTTP_BASE_HEADER_PARSER_H__

#include <utility>
#include "Common/Typedefs.h"
#include "Common/ErrorNo.h"
#include "StatusLine.h"
#include "Tracker/Trace.h"

using std::pair;

class CLazyBuffer;
class CHeaderField;
class CRequestLine;
class CHeaderParser
{
public:
    CHeaderParser(char* pData, size_t len, CLazyBuffer* pBuffer = NULL);
    ~CHeaderParser();

    size_t GetConsumedSize() { return m_ConsumedSize; }

    ErrorCode CreateRequestLine(CRequestLine** pRequestLine)
    {
        // TODO: Impl
        ASSERT(false);
        return EC_UNKNOWN;
    }

    ErrorCode CreateStatusLine(CStatusLine** pStatusLine)
    {
        *pStatusLine = reinterpret_cast<CStatusLine*>(
            BuildStartedLine(CStatusLine::CreateInstance1));
        return GetStandardErrorCode(ERROR_CODE);
    }

    ErrorCode BuildHeaderField(CHeaderField* pHeaderField);

    static bool IsTokenChar(char c);

private:
    void* BuildStartedLine(void* (*creator)(const char*, size_t, CLazyBuffer*));

    static ErrorCode DecodeHeaderField(char* pBegin, char* pEnd,
                                       char** pKey, char** pValue);

private:
    char* m_pData;
    size_t m_DataLength;
    CLazyBuffer* m_pBuffer;
    size_t m_ConsumedSize;

    static const char* s_pTokenDelimiters;
};

#endif