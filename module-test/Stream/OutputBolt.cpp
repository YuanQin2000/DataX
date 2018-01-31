/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "OutputBolt.h"
#include <stdio.h>
#include "Thread/ArrayDataFrames.h"

ArrayDataFrames* COutputBolt::Process(ArrayDataFrames* pIndata)
{
    for (size_t i = 0; i < pIndata->Count; ++i) {
        printf("%lu)", i + 1);
        DataFrame frame;
        pIndata->GetFrame(i, &frame);
        const char* pString = reinterpret_cast<const char*>(frame.GetData());
        for (size_t j = 0; j < frame.Length; ++j) {
            putchar(pString[j]);
        }
    }
    ArrayDataFrames::DeleteInstance(pIndata);
    return NULL;
}
