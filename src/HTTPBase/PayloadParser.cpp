/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "PayloadParser.h"
#include <cctype>
#include "Compress/CompressManager.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::isxdigit;

CPayloadEncoder::CPayloadEncoder(CompressType ct, bool bChunked) :
    m_CompressType(ct),
    m_bChunked(bChunked)
{
}

int CPayloadEncoder::Process(uint8_t* pData, size_t len)
{
    return 0;
}

void CPayloadEncoder::SetEnd()
{
    if (m_bChunked) {
        // TODO: Encode last chunk packet.
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// CChunkParser Class Implemenation
//
///////////////////////////////////////////////////////////////////////////////

/**
 *  chunked-body = *chunk
 *                  last-chunk
 *                  trailer-part
 *                  CRLF
 *                     
 *  chunk = chunk-size [ chunk-ext ] CRLF
 *          chunk-data CRLF
 *  chunk-data = 1*OCTET
 * 
 *  last-chunk  = 1*("0") [ chunk-ext ] CRLF
 * 
 *  chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
 *    chunk-ext-name = token
 *    chunk-ext-val  = token / quoted-string
 * 
 *  trailer-part   = *( header-field CRLF )
 */
ErrorCode CChunkParser::ProcessData(
    uint8_t* pBuffer, size_t len, size_t* pOutConsumed)
{
    ASSERT(len > 0);
    ASSERT(pOutConsumed);

    const char* pCur = reinterpret_cast<const char*>(pBuffer);
    const char* pEnd = pCur + len;

    *pOutConsumed = 0;
    m_pPayload = NULL;
    m_PayloadLen = 0;
    if (m_ChunkSize == CHUNK_SIZE_NOT_SET) {
        // A new chunk, Parse header first.
        const char* pChunkHeaderEnd = NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd);
        if (!pChunkHeaderEnd) {
            // Not received complete chunk header,Need more data to read
            return EC_INPROGRESS;
        }

        ErrorCode resErr = DecodeChunkHeader(pCur, pChunkHeaderEnd, m_ChunkSize);

#ifdef __DEBUG__
        OUTPUT_DEBUG_TRACE("Chunk size: %d\n", m_ChunkSize);
#endif

        *pOutConsumed += pChunkHeaderEnd + 2 - pCur;  // Add the CRLF.
        pCur = pChunkHeaderEnd + 2;
        if (resErr != EC_SUCCESS) {
            return resErr;
        }
        if (pCur == pEnd) {
            return EC_INPROGRESS;
        }
    }

    if (m_ChunkSize > 0) {  // Normal Data.
        ASSERT(static_cast<size_t>(m_ChunkSize) >= m_RecvBytes);
        size_t expectedDataLen = static_cast<size_t>(m_ChunkSize) - m_RecvBytes;
        const char* pChunkDataEnd = pEnd;
        if (pCur + expectedDataLen < pEnd) {
            pChunkDataEnd = pCur + expectedDataLen;
        }
        size_t recvDataLen = pChunkDataEnd - pCur;
        if (recvDataLen > 0) {
            m_pPayload = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(pCur));
            m_PayloadLen = recvDataLen;
            *pOutConsumed += recvDataLen;
            pCur += recvDataLen;
            m_RecvBytes += recvDataLen;
        }
        if (pCur + 1 < pEnd) {
            if (*pCur == '\r' && *(pCur + 1) == '\n') {
                m_ChunkSize = CHUNK_SIZE_NOT_SET;   // For next new chunk.
                m_RecvBytes = 0;
                *pOutConsumed += 2;
            } else {
                OUTPUT_ERROR_TRACE(
                    "Chunked data length NOT EQU the declared: received: %d, chunkSize: %d\n",
                    m_RecvBytes, m_ChunkSize);
                return EC_PROTOCOL_ERROR;
            }
        }
        return EC_INPROGRESS;
    }

    // Last chunk.
    const char* pChunkBodyEnd = NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd);
    if (!pChunkBodyEnd) {
        // Need more data to read
        return EC_INPROGRESS;
    }
    *pOutConsumed += pChunkBodyEnd + 2 - pCur;
    if (pChunkBodyEnd == pCur) {
        // Reach the end of the chunk.
        return EC_SUCCESS;
    }

    // To parse the tailer part.
    // TODO: Implemantion.
    ASSERT(false);
    return EC_UNKNOWN;
}

// TODO: To anaylze the chunk extension.
ErrorCode CChunkParser::DecodeChunkHeader(
    const char* pBegin, const char* pEnd, int& chunkSize)
{
    int size = 0;
    bool bFoundDigit = false;
    const char* pCur = pBegin;

    while (pCur < pEnd) {
        if (!isxdigit(*pCur)) {
            break;
        }
        int num = 0;
        char ch = *pCur;
        if (ch <= '9') {
            num = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            num = ch - 'a' + 10;
        } else {
            num = ch - 'A' + 10;
        }
        size = (size << 4) + num;
        ++pCur;
        bFoundDigit = true;
    }
    if (!bFoundDigit) {
        OUTPUT_WARNING_TRACE("Expect digital number but is %c(%d)\n", *pCur, *pCur);
        return EC_PROTOCOL_MALFORMAT;
    }
    chunkSize = size;
    // TODO: Check the chunk-extension
    return EC_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//
// CPayloadDecoder Class Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CPayloadDecoder::CPayloadDecoder(
    tPayloadCallBack cb, void* pCBOwner, int contentLen, CompressType type) :
    m_DataCB(cb),
    m_pDataCBOwner(pCBOwner),
    m_pChunkParser(NULL),
    m_pDecompressor(NULL),
    m_ContentLength(contentLen),
    m_CompressType(type),
    m_RecvBytes(0),
    m_bReceivedAll(false)
{
    ASSERT(cb);
    ASSERT(pCBOwner);
}

CPayloadDecoder::~CPayloadDecoder()
{
    delete m_pDecompressor;
    delete m_pChunkParser;
}

ErrorCode CPayloadDecoder::Process(
    uint8_t* pData, size_t dataLen, size_t* pConsumed)
{
    ASSERT(dataLen > 0);
    ASSERT(pConsumed);

    uint8_t* pPayload = pData;
    size_t payloadLen = 0;

    *pConsumed = 0;
    if (m_ContentLength == 0) { // No content
        return EC_SUCCESS;
    }

    ErrorCode resErr = EC_INPROGRESS;
    if (m_ContentLength > 0) {
        if (dataLen + m_RecvBytes >= static_cast<size_t>(m_ContentLength)) {
            // All payload received.
            payloadLen = m_ContentLength - m_RecvBytes;
            resErr = EC_SUCCESS;
        } else {
            payloadLen = dataLen;
        }
        *pConsumed = payloadLen;
    } else if  (m_ContentLength == CONTENT_LENGTH_UNKNOWN) {
        payloadLen = dataLen;
        *pConsumed = payloadLen;
    } else { // m_ContentLength == CONTENT_LENGTH_CHUNKED
        if (m_pChunkParser == NULL) {
            m_pChunkParser = new CChunkParser();
            if (m_pChunkParser == NULL) {
                return EC_NO_MEMORY;
            }
        }

        resErr = m_pChunkParser->ProcessData(pData, dataLen, pConsumed);
        pPayload = m_pChunkParser->GetPayload(&payloadLen);
    }

    m_bReceivedAll = (resErr == EC_SUCCESS);

    if (pPayload && payloadLen > 0) {
        m_RecvBytes += payloadLen;
        if (m_CompressType != CT_NONE) {
            ErrorCode error = Decompress(pPayload, payloadLen);
            if (error != EC_SUCCESS) {
                resErr = error;
            }
        } else {
            m_DataCB(m_pDataCBOwner, pPayload, payloadLen);
        }
    }
    return resErr;
}

ErrorCode CPayloadDecoder::Decompress(uint8_t* pData, size_t len)
{
    // Process the compressed data.
    if (!m_pDecompressor) {
        m_pDecompressor =
            CCompressManager::Instance()->CreateDecompressor(m_CompressType);
        if (!m_pDecompressor) {
            return EC_NO_MEMORY;
        }
    }

    CDynamicBuffer* pBuffer = m_pDecompressor->Process(pData, len);
    if (pBuffer) {
        CDynamicBuffer::DataBlock* pBlock = pBuffer->GetFirstBlock();
        while (pBlock) {
            ASSERT(pBlock->pData);
            if (pBlock->Length > 0) {
                m_DataCB(m_pDataCBOwner, pBlock->pData, pBlock->Length);
            }
            pBlock = pBlock->pNext;
        }
        CDynamicBuffer::DestroyInstance(pBuffer);
        pBuffer = NULL;
        return EC_SUCCESS;
    }

    switch (m_pDecompressor->ErrorCode()) {
    case CEC_NO_MEMORY: return EC_NO_MEMORY;
    case CEC_MALFORMAT: return EC_PROTOCOL_MALFORMAT;
    default:            return EC_UNKNOWN;
    }
}
