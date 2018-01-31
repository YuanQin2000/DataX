/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "OctetBuffer.h"
#include "Tracker/Trace.h"

COctetBuffer::COctetBuffer(
    uint8_t* pBuf, size_t bufLen, tBufferReleaser releaser /* = NULL */) :
    m_pData(pBuf),
    m_pFree(pBuf),
    m_pBufferBegin(pBuf),
    m_pBufferEnd(pBuf + bufLen),
    m_BufferSize(bufLen),
    m_FragmentMinSize(0),
    m_BufferReleaser(releaser)
{
    ASSERT(pBuf != NULL);
    ASSERT(bufLen != 0);

    unsigned int n = 4;
    size_t fragment = 0;
    while (n > 0) {
        fragment = m_BufferSize >> n;
        if (fragment > 0) {
            break;
        }
        --n;
    }
    ASSERT(fragment > 0);
    m_FragmentMinSize = fragment;
}

COctetBuffer::~COctetBuffer()
{
    if (m_BufferReleaser) {
        m_BufferReleaser(m_pBufferBegin);
    }
}

size_t COctetBuffer::Append(const uint8_t* pBuf, size_t len)
{
    size_t freeSize = GetTotalFreeSize();
    size_t toCopySize = len > freeSize ? freeSize : len;
    if (toCopySize > GetFreeBufferSize()) {
        size_t dataLength = GetDataLength();
        if (dataLength <= GetFrontFreeSize()) {
            memcpy(m_pBufferBegin, m_pData, dataLength);
        } else {
            memmove(m_pBufferBegin, m_pData, dataLength);
        }
        m_pData = m_pBufferBegin;
        m_pFree = m_pData + dataLength;
    }
    memcpy(m_pFree, pBuf, toCopySize);
    m_pFree += toCopySize;
    return toCopySize;
}

bool COctetBuffer::Append(uint8_t octect)
{
    bool bRes = false;

    if (GetFreeBufferSize() >= sizeof(octect)) {
        *m_pFree++ = octect;
        bRes = true;
    } else {
        size_t freeHole = GetFrontFreeSize();
        if (freeHole > sizeof(octect)) {
            size_t dataLength = GetDataLength();
            if (freeHole > dataLength) {
                memcpy(m_pBufferBegin, m_pData, dataLength);
            } else {
                memmove(m_pBufferBegin, m_pData, dataLength);
            }
            bRes = true;
        }
    }
    return bRes;
}

size_t COctetBuffer::SetPopOutLength(size_t len, bool bRelocationData /* = false */)
{
    ASSERT(m_pFree >= m_pData);

    size_t toPopSize = 0;
    if (len > 0) {
        size_t dataSize = GetDataLength();
        toPopSize = dataSize > len ? len : dataSize;
        m_pData += len;
    }
    if (bRelocationData) {
        RelocationDataIfNecessary();
    }
    return toPopSize;
}

void COctetBuffer::RelocationDataIfNecessary()
{
    size_t holeSize = m_pData - m_pBufferBegin;
    size_t dataLength = GetDataLength();

    if (dataLength < m_FragmentMinSize &&
        holeSize >= m_FragmentMinSize) {
        memcpy(m_pBufferBegin, m_pData, dataLength);
        m_pData = m_pBufferBegin;
        m_pFree -= holeSize;
    }
}

void COctetBuffer::RelocationData()
{
    if (m_pData > m_pBufferBegin) {
        size_t holeSize = m_pData - m_pBufferBegin;
        size_t dataLength = GetDataLength();
        if (dataLength > 0) {
            if (holeSize >= dataLength) {
                memcpy(m_pBufferBegin, m_pData, dataLength);
            } else {
                memmove(m_pBufferBegin, m_pData, dataLength);
            }
        }
        m_pData = m_pBufferBegin;
        m_pFree = m_pData + dataLength;
    }
}
