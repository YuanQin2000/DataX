/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAMUTILS_REMOTE_BOLT_H__
#define __STREAMUTILS_REMOTE_BOLT_H__

#include "Stream/Bolt.h"

class CConnection;
class CRemoteBolt : public IBolt
{
public:
    CRemoteBolt(CConnection& conn) : m_Connection(conn) {}
    ~CRemoteBolt() {}

    // From IBolt
    ArrayDataFrames* Process(ArrayDataFrames* pInData);

private:
    CConnection& m_Connection;
};

#endif