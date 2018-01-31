/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SimpleFilter.h"
#include "Common/CharHelper.h"
#include "Thread/ArrayDataFrames.h"
#include "Tracker/Trace.h"

ArrayDataFrames* CSimpleFilterBolt::Process(ArrayDataFrames* pIndata)
{
    ArrayDataFrames* pResult = NULL;
    if (pIndata->Count > 0) {
        pResult = ArrayDataFrames::CreateInstance(pIndata->Count);
    }

    if (pResult) {
        size_t dataIdx = 0;
        for (size_t i = 0; i < pIndata->Count; ++i) {
            DataFrame frame;
            pIndata->GetFrame(i, &frame);
            const char* pString = reinterpret_cast<const char*>(frame.GetData());
            if (frame.Length >= m_FilterLen) {
                if (NSCharHelper::FindSubStr(
                    m_pFilter, m_FilterLen, pString, pString + frame.Length)) {
                    pResult->SetFrame(dataIdx, &frame, false);
                    ++dataIdx;
                }
            }
        }
        if (dataIdx > 0) {
            pResult->ShrinkCount(dataIdx);
        } else {
            ArrayDataFrames::DeleteInstance(pResult);
            pResult = NULL;
        }
    }

    ArrayDataFrames::DeleteInstance(pIndata);
    return pResult;
}
