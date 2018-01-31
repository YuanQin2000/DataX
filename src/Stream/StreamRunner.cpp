/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "StreamRunner.h"
#include "BoltRunner.h"
#include "MsgDefs.h"
#include "Thread/ArrayDataFrames.h"

CStreamRunner::CStreamRunner(const char* pName, CBoltChain* pChain) :
    CDGNode(),
    m_pBoltChain(pChain)
{
    ASSERT(pName);

    int len = snprintf(m_Name, sizeof(m_Name) - 1, "strun-%p-%s", this, pName);
    ASSERT(len > 0);
    m_Name[len] = '\0';
}

CStreamRunner::~CStreamRunner()
{
    delete m_pBoltChain;
}

size_t CStreamRunner::ForwardMessage(ArrayDataFrames* pMsg)
{
    CForwardList& downStream(GetForwardNodes());
    size_t count = downStream.Count();
    if (count == 0) {
        return 0;
    }

    size_t index = 0;
    size_t sentCount = 0;
    CForwardList::Iterator iter = downStream.Begin();
    --count;
    while (index < count) {
        CBoltRunner* pBolt = reinterpret_cast<CBoltRunner*>(downStream.DataAt(iter));
        ArrayDataFrames* pNewMsg =
            reinterpret_cast<ArrayDataFrames*>(
                ArrayDataFrames::Duplicate(pMsg, pMsg->Count));
        if (pNewMsg) {
            ITMessage msg(STREAM_USER_DATA);
            msg.SetExtData(
                pNewMsg,
                pNewMsg->Count,
                ArrayDataFrames::DeleteInstance,
                ArrayDataFrames::Duplicate,
                true);
            if (pBolt->PushMessage(&msg)) {
                ++sentCount;
            } else {
                ArrayDataFrames::DeleteInstance(pNewMsg);
                OUTPUT_WARNING_TRACE("Push Message to bolt failed.\n");
            }
        }
        ++index;
        ++iter;
    }

    // Handle the last bolt.
    CBoltRunner* pLastBolt =
        reinterpret_cast<CBoltRunner*>(downStream.DataAt(iter));
    ITMessage msg(STREAM_USER_DATA);
    msg.SetExtData(
        pMsg,
        pMsg->Count,
        ArrayDataFrames::DeleteInstance,
        ArrayDataFrames::Duplicate,
        true);
    if (pLastBolt->PushMessage(&msg)) {
        ++sentCount;
    } else {
        ArrayDataFrames::DeleteInstance(pMsg);
        OUTPUT_WARNING_TRACE("Push Message to bolt failed.\n");
    }
    return sentCount;
}
