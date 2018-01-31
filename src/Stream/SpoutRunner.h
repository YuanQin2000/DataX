/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_SPOUT_RUNNER_H__
#define __STREAM_SPOUT_RUNNER_H__

#include "Common/Typedefs.h"
#include "StreamRunner.h"

class ISpout;
class IBolt;
class CBoltRunner;
class CThread;
class CSpoutRunner : public CStreamRunner
{
public:
    CSpoutRunner(const char* pName, ISpout& spout, CBoltChain* pChain);
    ~CSpoutRunner();

    // From CStreamRunner
    bool Start();
    bool Stop();

private:
    static void* Running(void* pArg);

private:
    CThread* m_pThread;
    ISpout& m_Spout;

    DISALLOW_DEFAULT_CONSTRUCTOR(CSpoutRunner);
    DISALLOW_COPY_CONSTRUCTOR(CSpoutRunner);
    DISALLOW_ASSIGN_OPERATOR(CSpoutRunner);
};

#endif