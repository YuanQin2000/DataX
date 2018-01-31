/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_COMPRESSOR_H__
#define __HTTP_COMPRESSOR_H__

#include "zlib/zlib.h"
#include "Compressor.h"

///////////////////////////////////////////////////////////////////////////////
//
// CGzipCompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CGzipCompressWrapper : public CCompressor
{
public:
    ~CGzipCompressWrapper();

    // From CCompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CCompressor* CreateInstance();

private:
    CGzipCompressWrapper();

private:
    z_stream m_Stream;
};


///////////////////////////////////////////////////////////////////////////////
//
// CGzipDecompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CGzipDecompressWrapper : public CDecompressor
{
public:
    ~CGzipDecompressWrapper();

    // From CDecompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CDecompressor* CreateInstance();

protected:
    CGzipDecompressWrapper();

private:
    z_stream m_Stream;
};

#endif