/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "LZWWrapper.h"

///////////////////////////////////////////////////////////////////////////////
//
// CLzwCompressWrapper Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CLzwCompressWrapper::CLzwCompressWrapper() :
    CCompressor(CT_COMPRESS)
{
}

CLzwCompressWrapper::~CLzwCompressWrapper()
{
}

CCompressor* CLzwCompressWrapper::CreateInstance()
{
    return NULL;
}

CDynamicBuffer* CLzwCompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLzwDecompressWrapper Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CLzwDecompressWrapper::CLzwDecompressWrapper() :
    CDecompressor(CT_COMPRESS)
{
}

CLzwDecompressWrapper::~CLzwDecompressWrapper()
{
}

CDecompressor* CLzwDecompressWrapper::CreateInstance()
{
    return NULL;
}

CDynamicBuffer* CLzwDecompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    return NULL;
}
