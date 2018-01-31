/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ArrayDataFrames.h"
#include <cstdlib>

using std::malloc;
using std::memset;
using std::free;

ArrayDataFrames* ArrayDataFrames::CreateInstance(size_t count)
{
    ASSERT(count > 0);

    size_t size = sizeof(ArrayDataFrames) + sizeof(DataFrame) * count;
    ArrayDataFrames* pInstance = reinterpret_cast<ArrayDataFrames*>(malloc(size));
    if (pInstance) {
        memset(pInstance, 0, size);
        pInstance->Count = count;
    }
    return pInstance;
}

void ArrayDataFrames::DeleteInstance(void* pData)
{
    ASSERT(pData);

    ArrayDataFrames* pFrames = reinterpret_cast<ArrayDataFrames*>(pData);
    for (size_t i = 0; i < pFrames->Count; ++i) {
        DataFrame* pDataFrame = &pFrames->Frames[i];
        pDataFrame->CleanData();
    }
    free(pData);
}

void* ArrayDataFrames::Duplicate(void* pData, size_t len)
{
    ArrayDataFrames* pFrames = reinterpret_cast<ArrayDataFrames*>(pData);

    ASSERT(pFrames->IsValid());
    ASSERT(len <= pFrames->Count);

    ArrayDataFrames* pDupFrames = CreateInstance(len);
    if (pDupFrames) {
        bool bFailed = false;
        for (size_t i = 0; i < len; ++i) {
            if (!pDupFrames->SetFrame(i, &pFrames->Frames[i])) {
                bFailed = true;
                break;
            }
            ASSERT(!pDupFrames->Frames[i].IsNull(), "index: %d\n", i);
        }
        if (bFailed) {
            DeleteInstance(pDupFrames);
            pDupFrames = NULL;
        }
    }
    return pDupFrames;
}

bool ArrayDataFrames::SetFrame(size_t idx, DataFrame* pDesc, bool bDeepCopy /* = true */)
{
    ASSERT(idx < Count);

    DataFrame* pFrame = &Frames[idx];
    bool bRes = true;
    if (bDeepCopy) {
        bRes = pFrame->DeepCopy(pDesc);
    } else {
        pFrame->ShallowCopy(pDesc);
    }
    return bRes;
}
