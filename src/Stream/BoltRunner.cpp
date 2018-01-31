/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "BoltRunner.h"
#include "BoltChain.h"
#include "Thread/ArrayDataFrames.h"
#include "Thread/ITMessage.h"
#include "Thread/Looper.h"
#include "Tracker/Trace.h"

CBoltRunner::CBoltRunner(
    const char* pName, CBoltChain* pChain, size_t msgBufSize /* = 0 */) :
    CStreamRunner(pName, pChain),
    m_pLoop(NULL),
    m_MsgSwitch(msgBufSize),
    m_RunnerObject(this)
{
}

CBoltRunner::~CBoltRunner()
{
    Stop();
}

bool CBoltRunner::Start()
{
    bool bRes = true;
    if (m_pLoop == NULL) {
        m_pLoop = CLooper::CreateInstance(GetName(), m_RunnerObject, m_MsgSwitch);
        if (m_pLoop == NULL) {
            OUTPUT_WARNING_TRACE("Create LoopRunner failed.\n");
            bRes = false;
        }
    }
    return bRes;
}

bool CBoltRunner::Stop()
{
    bool bRes = true;
    if (m_pLoop) {
        bRes = m_pLoop->Exit();
        if (bRes) {
            delete m_pLoop;
        }
    }
    return bRes;
}

void CBoltRunner::CRunnerObject::HandleMessage(ITMessage* pMsg)
{
    ArrayDataFrames* pFrames = reinterpret_cast<ArrayDataFrames*>(pMsg->GetData());
    ArrayDataFrames* pNewFrames = m_pContext->Process(pFrames);
    pMsg->SetOwnedExtData(false);   // take ownership of the message payload.
    if (pNewFrames) {
        size_t count = m_pContext->ForwardCount();
        ASSERT(count > 0, "Create message but no down stream handler.\n");
        size_t sentCount = m_pContext->ForwardMessage(pNewFrames);
        if (count != sentCount) {
            OUTPUT_WARNING_TRACE(
                "Forward to %d bolt runner while have %d count.\n", sentCount, count);
        }
    }
}
