/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DataFrame.h"
#include "Common/Macros.h"
#include "Tracker/Trace.h"
#include "Thread/Thread.h"
#include <cstdlib>

using std::malloc;
using std::free;

void FreeOnHeap(void* pData)
{
    free(pData);
}

void* CopyOnHeap(void* pData, size_t len)
{
    ASSERT(len > 0);

    void* pMem = malloc(len);
    if (pMem) {
        memcpy(pMem, pData, len);
    }
    return pMem;
}

void DataFrame::CleanData()
{
    if (TEST_FLAG(Flags, USED_EXTERNAL_DATA)) {
        if (TEST_FLAG(Flags, OWNED_EXTERNAL_DATA)) {
            Payload.Clean();
        }
        CLEAR_FLAG(Flags, USED_EXTERNAL_DATA);        
    }
    CLEAR_FLAG(Flags, OWNED_EXTERNAL_DATA);
    Length = 0;
}

void DataFrame::SetOwnedExtData(bool bOwned)
{
    if (bOwned) {
        if (TEST_FLAG(Flags, USED_EXTERNAL_DATA) &&
            Payload.ExtData.pExtData && Payload.ExtData.Cleaner) {
            SET_FLAG(Flags, OWNED_EXTERNAL_DATA);
        }
    } else {
        CLEAR_FLAG(Flags, OWNED_EXTERNAL_DATA);
    }
}

void DataFrame::SetExtData(
    void* pData,
    size_t len /* = 0 */,
    tDataCleaner cleaner /* = NULL */,
    tDataCloner cloner /* = NULL */,
    bool bOwned /* = false */)
{
    CleanData();
    Payload.SetExtData(pData, cleaner, cloner);
    Length = len;
    SET_FLAG(Flags, USED_EXTERNAL_DATA);
    if (bOwned) {
        SET_FLAG(Flags, OWNED_EXTERNAL_DATA);
    } else {
        CLEAR_FLAG(Flags, OWNED_EXTERNAL_DATA);
    }
}

bool DataFrame::DeepCopy(DataFrame* pMsg)
{
    if (TEST_FLAG(pMsg->Flags, USED_EXTERNAL_DATA)) {
        bool bOwned = false;
        void* pNew = pMsg->Payload.ExtData.pExtData;
        ASSERT(pNew, "Copy from a NULL frame\n");
        if (pMsg->Payload.ExtData.Cloner) {
            pNew = pMsg->Payload.ExtData.Cloner(
                pMsg->Payload.ExtData.pExtData, pMsg->Length);
            if (pNew == NULL) {
                OUTPUT_WARNING_TRACE("Clone data failed.\n");
                return false;
            }
            bOwned = true;
        }
        SetExtData(pNew,
            pMsg->Length,
            pMsg->Payload.ExtData.Cleaner,
            pMsg->Payload.ExtData.Cloner,
            bOwned);
        MsgID = pMsg->MsgID;
        Length = pMsg->Length;
    } else {
        CleanData();
        memcpy(this, pMsg, sizeof(DataFrame));
    }
    return true;
}

bool DataFrame::SetData(void* pData, size_t len)
{
    ASSERT(len > 0);

    if (len > sizeof(DataField)) {
        // Set as the external data (malloc on heap)
        void* pCopy = CopyOnHeap(pData, len);
        if (pCopy == NULL) {
            OUTPUT_WARNING_TRACE("copy %d bytes on heap failed.\n", len);
            return false;
        }
        CleanData();
        SetExtData(pCopy, len, FreeOnHeap, CopyOnHeap, true);
    } else {
        CleanData();
        memcpy(Payload.Buffer, pData, len);
    }
    Length = len;
    return true;
}

void* DataFrame::GetData(bool* pOutIsExt /* = NULL */)
{
    void* pData = NULL;
    if (TEST_FLAG(Flags, USED_EXTERNAL_DATA)) {
        if (pOutIsExt) {
            *pOutIsExt = true;
        }
        pData = Payload.ExtData.pExtData;
    } else if (Length > 0) {
        if (pOutIsExt) {
            *pOutIsExt = false;
        }
        pData = Payload.Buffer;
    }
    return pData;
}
