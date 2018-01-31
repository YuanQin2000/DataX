/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MODULE_TEST_STREAM_OUTPUT_H__
#define __MODULE_TEST_STREAM_OUTPUT_H__
#include "Stream/Bolt.h"

class COutputBolt : public IBolt
{
public:
    COutputBolt() {}
    ~COutputBolt() {}

    ArrayDataFrames* Process(ArrayDataFrames* pInData);
};

#endif