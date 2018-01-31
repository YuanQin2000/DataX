/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "DynamicBuffer.h"

enum CompressType {
    CT_NONE = -1,
    CT_COMPRESS,    // compress/x-compress coding, LZW
    CT_DEFLAT,      // deflat coding, LZ77
    CT_GZIP,        // GZip coding, LZ77 & CRC
    CT_COUNT
};

enum CompressErrorCode {
    CEC_SUCCESS,
    CEC_NO_MEMORY,
    CEC_MALFORMAT,
};

class CCompressor
{
public:
    CCompressor(CompressType type) :
        m_Error(CEC_SUCCESS), m_Type(type)
    {
        ASSERT(type >= 0 && type < CT_COUNT);
    }

    virtual ~CCompressor() {}

    CompressType Type()           const { return m_Type;  }
    CompressErrorCode ErrorCode() const { return m_Error; }

    virtual CDynamicBuffer* Process(uint8_t* pIn, size_t inLen) = 0;

protected:
    CompressErrorCode  m_Error;

private:
    const CompressType m_Type;

    DISALLOW_COPY_CONSTRUCTOR(CCompressor);
    DISALLOW_ASSIGN_OPERATOR(CCompressor);
    DISALLOW_DEFAULT_CONSTRUCTOR(CCompressor);
};


class CDecompressor
{
public:
    CDecompressor(CompressType type) :
        m_Error(CEC_SUCCESS), m_Type(type)
    {
        ASSERT(type >= 0 && type < CT_COUNT);
    }

    virtual ~CDecompressor() { }

    CompressType Type()           const { return m_Type;  }
    CompressErrorCode ErrorCode() const { return m_Error; }

    virtual CDynamicBuffer* Process(uint8_t* pIn, size_t inLen) = 0;


protected:
    CompressErrorCode  m_Error;

private:
    const CompressType m_Type;

    DISALLOW_COPY_CONSTRUCTOR(CDecompressor);
    DISALLOW_ASSIGN_OPERATOR(CDecompressor);
    DISALLOW_DEFAULT_CONSTRUCTOR(CDecompressor);
};

#endif