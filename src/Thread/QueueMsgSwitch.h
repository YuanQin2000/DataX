/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_QUEUE_MSG_SWITCH_H__
#define __THREAD_QUEUE_MSG_SWITCH_H__

#include "Common/Typedefs.h"
#include "Memory/MemoryPool.h"
#include "Common/ForwardList.h"
#include "ITMessage.h"
#include "Thread.h"
#include "Looper.h"
#include "Lock.h"
#include "Condition.h"

class CQueueMsgSwitch : public CMsgSwitch
{
public:
    CQueueMsgSwitch(size_t queueSize = 0);
    virtual ~CQueueMsgSwitch();

    bool ReadMessage(ITMessage* pOutMsg, int timeout = -1);

    /**
     * @warning The ownership of the message payload will be transferred
     */
    bool WriteMessage(ITMessage* pMsg);

private:
    CCriticalSection m_MsgCS;
    CCondition m_WriteCond;
    CCondition m_ReadCond;
    CMemoryPool m_MemoryPool;
    CForwardList m_MsgQueue;
    size_t m_MaxCount;

    DISALLOW_COPY_CONSTRUCTOR(CQueueMsgSwitch);
    DISALLOW_ASSIGN_OPERATOR(CQueueMsgSwitch);
};

#endif