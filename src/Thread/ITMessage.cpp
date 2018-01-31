/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ITMessage.h"

void ITMessage::Destroy()
{
    CleanData();
    if (TEST_FLAG(Flags, OWNED_SYNC_CONDITION)) {
        delete pSyncCond;
        pSyncCond = NULL;
        CLEAR_FLAG(Flags, OWNED_SYNC_CONDITION);
    }
}

void ITMessage::ShallowCopy(ITMessage* pMsg)
{
    DataFrame::ShallowCopy(pMsg);
    if (TEST_FLAG(Flags, OWNED_SYNC_CONDITION)) {
        delete pSyncCond;
        CLEAR_FLAG(Flags, OWNED_SYNC_CONDITION);
    }
    pSyncCond = pMsg->pSyncCond;
}

bool ITMessage::DeepCopy(ITMessage* pMsg)
{
    if (DataFrame::DeepCopy(pMsg)) {
        if (TEST_FLAG(Flags, OWNED_SYNC_CONDITION)) {
            delete pSyncCond;
            CLEAR_FLAG(Flags, OWNED_SYNC_CONDITION);
        }
        pSyncCond = pMsg->pSyncCond;
        return true;
    }
    return false;
}

bool ITMessage::SetSync(bool bSync)
{
    bool bRes = true;
    if (bSync) {
        if (pSyncCond == NULL) {
            pSyncCond = new CCondition();
            if (pSyncCond) {
                SET_FLAG(Flags, OWNED_SYNC_CONDITION);
            } else {
                bRes = false;
            }
        }
    } else {
        if (TEST_FLAG(Flags, OWNED_SYNC_CONDITION)) {
            delete pSyncCond;
        }
        pSyncCond = NULL;
    }
    return bRes;
}
