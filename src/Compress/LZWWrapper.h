/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMPRESS_LZW_WRAPPER_H__
#define __COMPRESS_LZW_WRAPPER_H__

#include "Compressor.h"

///////////////////////////////////////////////////////////////////////////////
//
// CLzwCompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CLzwCompressWrapper : public CCompressor
{
public:
    ~CLzwCompressWrapper();

    // From CCompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CCompressor* CreateInstance();

protected:
    CLzwCompressWrapper();
};


///////////////////////////////////////////////////////////////////////////////
//
// CLzwDecompressWrapper Declaration
//
///////////////////////////////////////////////////////////////////////////////
class CLzwDecompressWrapper : public CDecompressor
{
public:
    ~CLzwDecompressWrapper();

    // From CDecompressor
    CDynamicBuffer* Process(uint8_t* pIn, size_t inLen);

    static CDecompressor* CreateInstance();

private:
    CLzwDecompressWrapper();
};

#endif
