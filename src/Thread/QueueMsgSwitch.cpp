/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "QueueMsgSwitch.h"
#include <cstdlib>
#include <time.h>
#include <errno.h>

using std::memcpy;

CQueueMsgSwitch::CQueueMsgSwitch(size_t msgBufSize /* = 0 */) :
    CMsgSwitch(),
    m_MsgCS(),
    m_WriteCond(),
    m_ReadCond(),
    m_MemoryPool(CForwardList::ListNodeSize(sizeof(ITMessage))),
    m_MsgQueue(sizeof(ITMessage), &m_MemoryPool),
    m_MaxCount(msgBufSize)
{
    if (msgBufSize == 0) {
        m_MaxCount = static_cast<size_t>(-1);
    }
}

CQueueMsgSwitch::~CQueueMsgSwitch()
{
}

bool CQueueMsgSwitch::ReadMessage(ITMessage* pOutMsg, int timeout /* = -1 */)
{
    m_MsgCS.Lock();
    if (m_MsgQueue.Count() == 0) {
        m_ReadCond.Wait(&m_MsgCS, timeout);
        if (m_MsgQueue.Count() == 0) {
            m_ReadCond.Wait(&m_MsgCS, timeout);
            ASSERT(m_MsgQueue.Count() > 0);
            ASSERT(m_MsgQueue.First());
        }
    }

    // Consumed the full queue, to notify the blocking producer.
    bool bNotify = (m_MsgQueue.Count() == m_MaxCount);
    memcpy(pOutMsg, m_MsgQueue.First(), sizeof(ITMessage));
    m_MsgQueue.PopFront();
    if (bNotify) {
        m_WriteCond.Signal(&m_MsgCS);
    }
    m_MsgCS.Unlock();
    return true;
}

bool CQueueMsgSwitch::WriteMessage(ITMessage* pMsg)
{
    m_MsgCS.Lock();
    if (m_MsgQueue.Count() == m_MaxCount) {
        m_WriteCond.Wait(&m_MsgCS);
        ASSERT(m_MsgQueue.Count() < m_MaxCount);
    }
    bool bNotify = (m_MsgQueue.Count() == 0);
    m_MsgQueue.PushBack(pMsg);
    if (bNotify) {
        m_ReadCond.Signal(&m_MsgCS);
    }
    m_MsgCS.Unlock();
    return true;
}
