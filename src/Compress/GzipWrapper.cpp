/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "GzipWrapper.h"
#include <cstdlib>
#include <cstring>
#include "CompressManager.h"
#include "DynamicBuffer.h"
#include "Tracker/Trace.h"

using std::memset;
using std::malloc;
using std::free;


///////////////////////////////////////////////////////////////////////////////
//
// CGzipCompressWrapper Implemenation
//
// GZIP use little endian.
///////////////////////////////////////////////////////////////////////////////
CGzipCompressWrapper::CGzipCompressWrapper() :
    CCompressor(CT_GZIP)
{
    memset(&m_Stream, 0, sizeof(m_Stream));
}

CGzipCompressWrapper::~CGzipCompressWrapper()
{
    deflateEnd(&m_Stream);
}

CCompressor* CGzipCompressWrapper::CreateInstance()
{
    CGzipCompressWrapper* pInstance = new CGzipCompressWrapper();
    if (pInstance) {
        // allocate inflate state
        pInstance->m_Stream.zalloc = Z_NULL;
        pInstance->m_Stream.zfree = Z_NULL;
        pInstance->m_Stream.opaque = Z_NULL;
        if (deflateInit(&pInstance->m_Stream, -1) != Z_OK) {
            OUTPUT_WARNING_TRACE("deflateInit failed.\n");
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

CDynamicBuffer* CGzipCompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    ASSERT(false);  // TODO: Implementation
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CGzipDecompressWrapper Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CGzipDecompressWrapper::CGzipDecompressWrapper() :
    CDecompressor(CT_GZIP)
{
    memset(&m_Stream, 0, sizeof(m_Stream));
}

CGzipDecompressWrapper::~CGzipDecompressWrapper()
{
    inflateEnd(&m_Stream);
}

CDecompressor* CGzipDecompressWrapper::CreateInstance()
{
    CGzipDecompressWrapper* pInstance = new CGzipDecompressWrapper();
    if (pInstance) {
        // allocate inflate state
        pInstance->m_Stream.zalloc = Z_NULL;
        pInstance->m_Stream.zfree = Z_NULL;
        pInstance->m_Stream.opaque = Z_NULL;
        pInstance->m_Stream.avail_in = 0;
        pInstance->m_Stream.next_in = Z_NULL;
        if (inflateInit2(&pInstance->m_Stream, 47) != Z_OK) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

CDynamicBuffer* CGzipDecompressWrapper::Process(uint8_t* pIn, size_t inLen)
{
    ASSERT(m_Stream.avail_in == 0);
    m_Stream.avail_in = inLen;
    m_Stream.next_in = pIn;

    CDynamicBuffer* pBuffer = CDynamicBuffer::CreateInstance(inLen);
    if (pBuffer) {
        int res;
        CDynamicBuffer::DataBlock* pCurrentBlock = pBuffer->GetFirstBlock();
        bool bContinued = false;

        do {
            m_Stream.avail_out = inLen;
            m_Stream.next_out = pCurrentBlock->pData;
            res = inflate(&m_Stream, Z_NO_FLUSH);
            switch (res) {
            case Z_ERRNO:
            case Z_STREAM_ERROR:
                ASSERT(false);
                break;

            case Z_NEED_DICT:
            case Z_DATA_ERROR:
                m_Error = CEC_MALFORMAT;
                break;

            case Z_MEM_ERROR:
                m_Error = CEC_NO_MEMORY;
                break;
            }

            if (m_Error != CEC_SUCCESS) {
                OUTPUT_ERROR_TRACE("inflate return %d\n", res);
                CDynamicBuffer::DestroyInstance(pBuffer);
                return NULL;
            }

            if (res == Z_STREAM_END) {
                if (m_Stream.avail_in == 0) {
                    pCurrentBlock->Length = inLen - m_Stream.avail_out;
                    return pBuffer;
                }
                m_Error = CEC_MALFORMAT;
            }

            if (m_Error != CEC_SUCCESS) {
                CDynamicBuffer::DestroyInstance(pBuffer);
                return NULL;
            }

            if (m_Stream.avail_out == 0) {
                // Not finished, continue doing
                pCurrentBlock = pBuffer->CreateBlock(inLen);
                if (!pCurrentBlock) {
                    m_Error = CEC_NO_MEMORY;
                    CDynamicBuffer::DestroyInstance(pBuffer);
                    return NULL;
                }
                bContinued = true;
            } else {
                bContinued = false;
                pCurrentBlock->Length = inLen - m_Stream.avail_out;
            }
        } while (bContinued);
    }

    return pBuffer;
}
