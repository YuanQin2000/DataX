/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_IT_MESSAGE_H__
#define __THREAD_IT_MESSAGE_H__

#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "DataFrame.h"
#include "Condition.h"
#include "Tracker/Trace.h"
#include <cstring>

using std::memcpy;

#define MAX_SYSTEM_MSGID 100
#define USER_MSGID_BEGIN MAX_SYSTEM_MSGID + 1


/**
 * @brief ITMessage: Inter Thread Message.
 *        Message for the information exchange inter threads
 * @see IPMessage (Inter Process Message)
 */
struct ITMessage : public DataFrame {
    CCondition* pSyncCond;

    ITMessage(
        uint8_t msgID = 0, CCondition* pCond = NULL, bool bOwnedCond = false) :
        DataFrame(msgID), pSyncCond(pCond)
    {
        if (pCond && bOwnedCond) {
            SET_FLAG(Flags, OWNED_SYNC_CONDITION);
        }
    }

    void WaitSink()
    {
        ASSERT(pSyncCond);
        pSyncCond->Wait(NULL);
    }

    void SignalSourceIfNeeded()
    {
        if (pSyncCond) {
            pSyncCond->Signal(NULL);
        }
    }

    void ShallowCopy(ITMessage* pMsg);
    bool DeepCopy(ITMessage* pMsg);
    bool SetSync(bool bSync);
    void Destroy();

    static const uint8_t OWNED_SYNC_CONDITION = 0x04;
};

#endif
