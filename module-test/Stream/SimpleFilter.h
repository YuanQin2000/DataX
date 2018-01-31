/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MODULE_TEST_STREAM_SIMPLE_FILTER_H__
#define __MODULE_TEST_STREAM_SIMPLE_FILTER_H__

#include "Common/Typedefs.h"
#include "Stream/Bolt.h"
#include "Tracker/Trace.h"
#include <cstring>

using std::strlen;

class CSimpleFilterBolt : public IBolt
{
public:
    CSimpleFilterBolt(const char* pStr) : m_pFilter(pStr), m_FilterLen(strlen(pStr))
    {
        ASSERT(pStr);
        ASSERT(m_FilterLen > 0);
    }

    ~CSimpleFilterBolt() {}

    ArrayDataFrames* Process(ArrayDataFrames* pIndata);

private:
    const char* m_pFilter;
    size_t m_FilterLen;
};

#endif
