/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DeflatWrapper.h"

///////////////////////////////////////////////////////////////////////////////
//
// CDeflatCompressWrapper Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CDeflatCompressWrapper::CDeflatCompressWrapper() :
    CCompressor(CT_DEFLAT)
{
}

CDeflatCompressWrapper::~CDeflatCompressWrapper()
{
}

CCompressor* CDeflatCompressWrapper::CreateInstance()
{
    return NULL;
}

CDynamicBuffer* CDeflatCompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CDeflatDecompressWrapper Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CDeflatDecompressWrapper::CDeflatDecompressWrapper() :
    CDecompressor(CT_DEFLAT)
{
}

CDeflatDecompressWrapper::~CDeflatDecompressWrapper()
{
}

CDecompressor* CDeflatDecompressWrapper::CreateInstance()
{
    return NULL;
}

CDynamicBuffer* CDeflatDecompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    return NULL;
}
