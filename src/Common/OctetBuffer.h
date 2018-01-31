/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_OCTET_BUFFER_H__
#define __COMMON_OCTET_BUFFER_H__

#include <cstring>
#include <stdlib.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

class COctetBuffer
{
public:
    typedef void (*tBufferReleaser)(void* pBuf);

    COctetBuffer(uint8_t* pBuf, size_t bufLen, tBufferReleaser releaser = NULL);
    ~COctetBuffer();

    uint8_t* GetData() const { return m_pData; }
    size_t GetDataLength() const { return m_pFree - m_pData; }
    uint8_t* GetFreeBuffer() const { return m_pFree < m_pBufferEnd ? m_pFree : NULL; }
    size_t GetFreeBufferSize() const { return m_pFree < m_pBufferEnd ? m_pBufferEnd - m_pFree : 0; }

    size_t GetTotalFreeSize() const { return m_BufferSize - GetDataLength(); }
    size_t GetFrontFreeSize() const { return m_pData - m_pBufferBegin; }

    void Reset() { m_pData = m_pFree = m_pBufferBegin; }

    void SetPushInLength(size_t len)
    {
        ASSERT(len <= GetFreeBufferSize(), "Push %d\n", len);
        m_pFree += len;
    }

    size_t SetPopOutLength(size_t len, bool bRelocationData = false);

    size_t Append(const uint8_t* pBuf, size_t len);
    bool Append(uint8_t octect);

    void RelocationDataIfNecessary();
    void RelocationData();

private:
    uint8_t* m_pData;
    uint8_t* m_pFree;
    uint8_t* m_pBufferBegin;
    uint8_t* m_pBufferEnd;
    const size_t m_BufferSize;
    size_t m_FragmentMinSize;
    tBufferReleaser m_BufferReleaser;

    DISALLOW_COPY_CONSTRUCTOR(COctetBuffer);
    DISALLOW_ASSIGN_OPERATOR(COctetBuffer);
    DISALLOW_DEFAULT_CONSTRUCTOR(COctetBuffer);
};

#endif