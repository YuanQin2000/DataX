/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Source.h"
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Sink.h"
#include "Tracker/Trace.h"

CSource::CSource()
{
}

CSource::~CSource()
{
    if (m_pSink) {
        m_pSink->DisconnectSource();
    }
}

void CSource::OnConnected(CSink* pSink)
{
    ASSERT(pSink);
    ASSERT(!m_pSink);

    m_pSink = pSink;
    if (IsReadable()) {
        m_pSink->OnSourceReady();
    }
}

void CSource::OnDisconnected(CSink* pSink)
{
    ASSERT(pSink);
    ASSERT(m_pSink);
    ASSERT(m_pSink == pSink);

    m_pSink = NULL;
}


CMemorySource::CMemorySource(uint8_t* data, size_t len)
{
    ASSERT(data && len > 0);
    m_pData = data;
    m_pCurrent = m_pData;
    m_Length = len;
}

CMemorySource::~CMemorySource()
{
}

int CMemorySource::Read(uint8_t* to, size_t size)
{
    ASSERT(to && size > 0);

    size_t copied = 0;
    if (m_pData + m_Length > m_pCurrent) {
        copied = m_pData + m_Length - m_pCurrent;
    }
    if (copied > 0) {
        if (copied > size) {
            copied = size;
        }
        std::memcpy(to, m_pCurrent, copied);
        m_pCurrent = m_pData + copied;
    }
    return copied;
}
