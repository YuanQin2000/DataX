/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_BOLT_RUNNER_H__
#define __STREAM_BOLT_RUNNER_H__

#include "StreamRunner.h"
#include "MsgDefs.h"
#include "Common/Typedefs.h"
#include "Thread/ArrayDataFrames.h"
#include "Thread/QueueMsgSwitch.h"
#include "Tracker/Trace.h"

class IBolt;
class CLooper;

class CBoltRunner : public CStreamRunner
{
public:
    CBoltRunner(const char* pName, CBoltChain* pChain, size_t msgBufSize = 0);
    ~CBoltRunner();

    // From CStreamRunner
    bool Start();
    bool Stop();

    bool PushMessage(ITMessage* pMsg, bool bSync = false)
    {
        return bSync ?
            m_MsgSwitch.WriteMessageSync(pMsg) :
            m_MsgSwitch.WriteMessage(pMsg);
    }

private:
    class CRunnerObject : public IRunner
    {
    public:
        CRunnerObject(CBoltRunner* pCxt) : m_pContext(pCxt) { ASSERT(pCxt); }

        // From IRunner
        void HandleMessage(ITMessage* pMsg);

    private:
        CBoltRunner* m_pContext;
    };

private:
    CLooper* m_pLoop;
    CQueueMsgSwitch m_MsgSwitch;
    CRunnerObject m_RunnerObject;

    DISALLOW_DEFAULT_CONSTRUCTOR(CBoltRunner);
    DISALLOW_COPY_CONSTRUCTOR(CBoltRunner);
    DISALLOW_ASSIGN_OPERATOR(CBoltRunner);
};

#endif