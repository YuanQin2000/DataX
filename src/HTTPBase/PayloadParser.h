/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_PAYLOAD_PARSER_H__
#define __HTTP_BASE_PAYLOAD_PARSER_H__

#include "Common/ErrorNo.h"
#include "HeaderParser.h"
#include "Compress/Compressor.h"


#define CONTENT_LENGTH_CHUNKED -1
#define CONTENT_LENGTH_UNKNOWN -2


///////////////////////////////////////////////////////////////////////////////
//
// CPayloadEncoder Class Definitions
//
///////////////////////////////////////////////////////////////////////////////
// TODO:
class CPayloadEncoder
{
public:
    CPayloadEncoder(CompressType ct, bool bChunked);

    int Process(uint8_t* pData, size_t len);
    void SetEnd();

private:
    CompressType m_CompressType;
    bool m_bChunked;
};


///////////////////////////////////////////////////////////////////////////////
//
// CChunkParser Class Definitions
//
///////////////////////////////////////////////////////////////////////////////
class CChunkParser
{
public:
    CChunkParser() :
        m_pPayload(NULL),
        m_PayloadLen(0),
        m_ChunkSize(CHUNK_SIZE_NOT_SET),
        m_RecvBytes(0) {}
    ~CChunkParser() {}

    uint8_t* GetPayload(size_t* pOutLen)
    {
        if (pOutLen) {
            *pOutLen = m_PayloadLen;
        }
        return m_pPayload;
    }

    ErrorCode ProcessData(uint8_t* pBuffer, size_t len, size_t* pOutConsumed);

private:
    static ErrorCode DecodeChunkHeader(
        const char* pBegin, const char* pEnd, int& chunkSize);

    uint8_t* m_pPayload;
    size_t m_PayloadLen;
    int m_ChunkSize;
    size_t m_RecvBytes;

    static const int CHUNK_SIZE_NOT_SET = -1;
};


///////////////////////////////////////////////////////////////////////////////
//
// CPayloadDecoder Class Definitions
//
///////////////////////////////////////////////////////////////////////////////
class CPayloadDecoder
{
public:
    typedef void (*tPayloadCallBack)(void* pThis, uint8_t* pData, size_t length);

    CPayloadDecoder(
        tPayloadCallBack cb, void* pCBOwner, int contentLen, CompressType type);
    ~CPayloadDecoder();

    ErrorCode Process(uint8_t* pData, size_t dataLen, size_t* pConsumed);
    bool HandlePeerClosed() const
    {
        return m_bReceivedAll || m_ContentLength == CONTENT_LENGTH_UNKNOWN;
    }

private:
    ErrorCode Decompress(uint8_t* pData, size_t len);

private:
    tPayloadCallBack m_DataCB;
    void* m_pDataCBOwner;
    CChunkParser* m_pChunkParser;    // Owned
    CDecompressor* m_pDecompressor;  // Owned
    const int m_ContentLength;
    const CompressType m_CompressType;
    size_t m_RecvBytes;
    bool m_bReceivedAll;
};

#endif