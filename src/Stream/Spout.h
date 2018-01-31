/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_SPOUT_H__
#define __STREAM_SPOUT_H__

struct ArrayDataFrames;
class ISpout
{
public:
    virtual ~ISpout() {}

    virtual bool Read(ArrayDataFrames** pOutFrames) = 0;
};

#endif