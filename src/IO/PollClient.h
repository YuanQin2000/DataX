/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_POLL_CLIENT_H__
#define __IO_POLL_CLIENT_H__

#include "Common/Typedefs.h"
#include "Poller.h"
#include "Tracker/Trace.h"

class CPollClient
{
public:
    CPollClient(tIOHandle io, uint32_t eventMask) :
        m_hIO(io), m_EventMask(eventMask)
    {
        ASSERT(io != INVALID_IO_HANDLE);
        ASSERT(eventMask != 0);
    }

    virtual ~CPollClient() {}

    inline tIOHandle PollIO() const { return m_hIO; }
    inline uint32_t PollEventMask() const { return m_EventMask; }

    virtual void OnAttached(bool bSuccess) = 0;
    virtual void OnDetached() = 0;

    virtual void OnIncomingData() = 0;
    virtual void OnOutgoingReady() = 0;
    virtual void OnPeerClosed() = 0;

private:
    const tIOHandle m_hIO;  // Not owned
    const uint32_t m_EventMask;
};

#endif