/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_BOLT_H__
#define __STREAM_BOLT_H__

struct ArrayDataFrames;
class IBolt
{
public:
    virtual ~IBolt() {}

    /**
     * @brief Process the data.
     * @param pIndata The data to be processed.
     * @return The output data if generated, NULL otherwise.
     * @warning The Bolt object is response to release @pIndata
     */
    virtual ArrayDataFrames* Process(ArrayDataFrames* pInData) = 0;
};

#endif