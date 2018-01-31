/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_SINK_H__
#define __COMMON_SINK_H__

class CSource;
class CSink
{
public:
    CSink();
    virtual ~CSink();

public:
    void ConnectSource(CSource* pSource);
    void DisconnectSource();

    virtual void OnSourceReady() = 0;

protected:
    CSource* m_pSource;
};

#endif