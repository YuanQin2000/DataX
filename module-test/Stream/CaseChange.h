/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MODULE_TEST_STREAM_CASE_CHANGE_H__
#define __MODULE_TEST_STREAM_CASE_CHANGE_H__

#include "Common/Typedefs.h"
#include "Stream/Bolt.h"
#include "Tracker/Trace.h"

class CCaseChangeBolt : public IBolt
{
public:
    enum CaseChangeType {
        CCT_LOWER2UPPER,
        CCT_UPPER2LOWER,
        CCT_REVERSE
    };

    CCaseChangeBolt(CaseChangeType type) : m_Type(type) {}
    ~CCaseChangeBolt() {}

    ArrayDataFrames* Process(ArrayDataFrames* pInData);

private:
    CaseChangeType m_Type;
};

#endif
