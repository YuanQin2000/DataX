/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __THREAD_DATA_FRAME_H__
#define __THREAD_DATA_FRAME_H__

#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Tracker/Trace.h"
#include <cstring>

using std::memcpy;
using std::memset;

typedef void (*tDataCleaner)(void* pData);
typedef void* (*tDataCloner)(void* pData, size_t len);

extern void FreeOnHeap(void* pData);
extern void* CopyOnHeap(void* pData, size_t len);

struct ExternalData {
    void* pExtData;
    tDataCleaner Cleaner;
    tDataCloner Cloner;
};

union DataField {
    ExternalData ExtData;
    uint8_t Buffer[sizeof(ExternalData)];

    DataField()
    {
        memset(Buffer, 0, sizeof(Buffer));
    }

    void Clean()
    {
        if (ExtData.pExtData && ExtData.Cleaner) {
            ExtData.Cleaner(ExtData.pExtData);
            ExtData.pExtData = NULL;
        }
    }

    void SetExtData(void* pExt, tDataCleaner cleaner, tDataCloner cloner)
    {
        ExtData.pExtData = pExt;
        ExtData.Cleaner = cleaner;
        ExtData.Cloner = cloner;
    }
};

struct DataFrame {
    uint8_t MsgID;
    uint8_t Flags;
    uint16_t Length;
    DataField Payload;

    DataFrame(uint8_t id = 0, uint8_t flags = 0, uint16_t len = 0) :
        MsgID(id), Flags(flags), Length(len) {}

    void CleanData();
    void SetOwnedExtData(bool bOwned);
    void SetExtData(
        void* pData,
        size_t len = 0,
        tDataCleaner cleaner = NULL,
        tDataCloner cloner = NULL,
        bool bOwned = false);
    bool SetData(void* pData, size_t len);
    void* GetData(bool* pOutIsExt = NULL);
    bool DeepCopy(DataFrame* pMsg);

    void ShallowCopy(DataFrame* pData)
    {
        CleanData();
        memcpy(this, pData, sizeof(DataFrame));
        CLEAR_FLAG(Flags, OWNED_EXTERNAL_DATA);
    }

    bool IsNull()
    {
        if (TEST_FLAG(Flags, USED_EXTERNAL_DATA)) {
            return Payload.ExtData.pExtData == NULL;
        }
        return Length == 0;
    }

    static const uint8_t USED_EXTERNAL_DATA = 0x01;
    static const uint8_t OWNED_EXTERNAL_DATA = 0x02;
};


#endif