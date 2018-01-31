/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_DATA_FRAME_H__
#define __STREAM_DATA_FRAME_H__

#include "Common/Typedefs.h"
#include "Thread/DataFrame.h"
#include "Tracker/Trace.h"

struct ArrayDataFrames
{
    uint16_t Count;
    DataFrame Frames[0];

    void ShrinkCount(size_t cnt)
    {
        ASSERT(cnt <= Count);
        Count = cnt;
    }

    bool IsValid()
    {
        for (size_t i = 0; i < Count; ++i) {
            if (Frames[i].IsNull()) {
                return false;
            }
        }
        return true;
    }

    void GetFrame(size_t idx, DataFrame* pOutFrame)
    {
        ASSERT(idx < Count);
        pOutFrame->ShallowCopy(&Frames[idx]);
    }

    bool SetFrame(size_t idx, DataFrame* pDesc, bool bDeepCopy = true);

    static ArrayDataFrames* CreateInstance(size_t count);
    static void DeleteInstance(void* pData);
    static void* Duplicate(void* pData, size_t len);
};

#endif
