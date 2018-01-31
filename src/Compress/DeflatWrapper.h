/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMPRESS_DEFLAT_WRAPPER_H__
#define __COMPRESS_DEFLAT_WRAPPER_H__

#include "Compressor.h"

///////////////////////////////////////////////////////////////////////////////
//
// CDeflatCompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CDeflatCompressWrapper : public CCompressor
{
public:
    ~CDeflatCompressWrapper();

    // From CCompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CCompressor* CreateInstance();

protected:
    CDeflatCompressWrapper();
};


///////////////////////////////////////////////////////////////////////////////
//
// CDeflatDecompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CDeflatDecompressWrapper : public CDecompressor
{
public:
    ~CDeflatDecompressWrapper();

    // From CDecompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CDecompressor* CreateInstance();

protected:
    CDeflatDecompressWrapper();
};

#endif