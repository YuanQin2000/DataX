/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "IOContext.h"
#include <errno.h>
#include "Common/ErrorNo.h"
#include "IO/IOHelper.h"

size_t CIOContext::DoRead(void* pBuf, size_t len)
{
    size_t readLen = NSIOHelper::Read(m_hIO, pBuf, len);
    if (readLen == 0) {
        HandleError(ERROR_CODE);
    }
    return readLen;
}

size_t CIOContext::DoWrite(void* pBuf, size_t len)
{
    size_t writeLen = NSIOHelper::Write(m_hIO, pBuf, len);
    if (writeLen == 0) {
        HandleError(ERROR_CODE);
    }
    return writeLen;
}

void CIOContext::HandleError(int errorCode)
{
    switch (errorCode) {
    case 0:
        m_Status = IOS_OK;
        return;

    case EAGAIN:
    case EINPROGRESS:   // Stream Socket is in handshake.
//    case EWOULDBLOCK:
        m_Status = IOS_NOT_READY;
        return;

    case ECONNREFUSED:
    case ECONNRESET:
        m_Status = IOS_CLOSED;
        break;

    case ENETUNREACH:   // Network unreachable
    case EHOSTUNREACH:  // Host unreachable
        m_Status = IOS_UNREACHABLE;
        break;

    default:
        m_Status = IOS_LOCAL_ERROR;
        break;
    }
    OUTPUT_NOTICE_TRACE("IO error: %s(%d)\n", strerror(errorCode), errorCode);
}
