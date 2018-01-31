/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_CONTEXT_H__
#define __IO_CONTEXT_H__

#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include <unistd.h>

class CIOContext
{
public:
    enum IOStatus {
        IOS_OK,
        IOS_NOT_READY,
        IOS_LOCAL_ERROR,
        IOS_CLOSED,
        IOS_UNREACHABLE,
    };

public:
    CIOContext(
        bool bBlockMode, tIOHandle io = INVALID_IO_HANDLE) :
        m_Status(IOS_OK),
        m_hIO(io),
        m_bBlockMode(bBlockMode),
        m_bReadable(false),
        m_bWritable(false) {}

    virtual ~CIOContext() {}

    size_t Read(void* pBuf, size_t len)
    {
        size_t readBytes = DoRead(pBuf, len);
        if (m_Status != IOS_OK) {
            m_bReadable = false;
        }
        return readBytes;
    }

    size_t Write(void* pBuf, size_t len)
    {
        size_t writeBytes = DoWrite(pBuf, len);
        if (m_Status != IOS_OK) {
            m_bWritable = false;
        }
        return writeBytes;
    }

    IOStatus GetStatus() const { return m_Status; }
    tIOHandle GetHandle() const { return m_hIO; } 
    bool IsBlockMode() const { return m_bBlockMode; }
    bool IsReadable()   { return m_bReadable; }
    bool IsWritable()   { return m_bWritable; }

    void SetReadable()
    {
        if (m_Status == IOS_OK || m_Status == IOS_NOT_READY) {
            m_Status = IOS_OK;
            m_bReadable = true;
        }
    }

    void SetWritable()
    {
        if (m_Status == IOS_OK || m_Status == IOS_NOT_READY) {
            m_Status = IOS_OK;
            m_bWritable = true;
        }
    }

public:
    virtual bool Open() = 0;

    virtual void Close()
    {
        if (m_hIO != INVALID_IO_HANDLE) {
            close(m_hIO);
        }
        m_hIO = INVALID_IO_HANDLE;
    }


private:
    virtual size_t DoRead(void* pBuf, size_t len);
    virtual size_t DoWrite(void* pBuf, size_t len);

    void HandleError(int errorCode);

protected:
    IOStatus m_Status;
    tIOHandle m_hIO;

private:
    const bool m_bBlockMode;
    bool m_bReadable;
    bool m_bWritable;
};

#endif