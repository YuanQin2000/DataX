/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SpoutRunner.h"
#include <cstdlib>
#include <cstdio>
#include "Spout.h"
#include "MsgDefs.h"
#include "Thread/ArrayDataFrames.h"
#include "Thread/ITMessage.h"
#include "Thread/Thread.h"

CSpoutRunner::CSpoutRunner(
    const char* pName, ISpout& spout, CBoltChain* pChain) :
    CStreamRunner(pName, pChain),
    m_pThread(NULL),
    m_Spout(spout)
{
}

CSpoutRunner::~CSpoutRunner()
{
    Stop();
}

bool CSpoutRunner::Start()
{
    if (m_pThread == NULL) {
        m_pThread = CThread::CreateInstance(
            GetName(), Running, this, CThread::PRIORITY_NORMAL, true);
    }
    return m_pThread != NULL;
}

bool CSpoutRunner::Stop()
{
    bool bRes = true;
    if (m_pThread) {
        bRes = m_pThread->Cancel();
        if (bRes) {
            delete m_pThread;
            m_pThread = NULL;
        }
    }
    return bRes;
}

void* CSpoutRunner::Running(void* pArg)
{
    CSpoutRunner* pThis = reinterpret_cast<CSpoutRunner*>(pArg);
    bool bEof = false;
    while (!bEof) {
        pThis->m_pThread->CheckCancel();
        ArrayDataFrames* pMsg = NULL;
        bEof = pThis->m_Spout.Read(&pMsg);
        if (pMsg == NULL) {
            continue;
        }

        ArrayDataFrames* pNewMsg = pThis->Process(pMsg);
        if (pNewMsg == NULL) {
            continue;
        }

        size_t count = pThis->ForwardCount();
        ASSERT(count > 0, "Create message but no down stream handler.\n");
        size_t sentCount = pThis->ForwardMessage(pNewMsg);
        if (count != sentCount) {
            OUTPUT_WARNING_TRACE(
                "Forward to %d bolt runner while have %d count.\n", sentCount, count);
        }
    }
    return NULL;
}
