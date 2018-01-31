/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_TOPOLOGY_H__
#define __STREAM_TOPOLOGY_H__

#include "Common/Typedefs.h"
#include "Thread/Lock.h"
#include "Thread/Condition.h"
#include <map>

using std::map;

typedef int16_t tTopologyID;

#define TOPOLOGY_NULL_ID -1

#define DECLARE_SPOUT(topObj, name, id, spout, ...) \
    do { \
        bool bRes = topObj->DeclareSpout(name, id, spout, ##__VA_ARGS__, NULL); \
        ASSERT(bRes); \
    } while (0)

#define DECLARE_BOLT(topObj, name, id, upStreamId, ...) \
    do { \
        bool bRes = topObj->DeclareBolt(name, id, upStreamId, ##__VA_ARGS__, NULL); \
        ASSERT(bRes); \
    } while (0)

class IBolt;
class ISpout;
class CBoltChain;
class CSpoutRunner;
class CBoltRunner;
class CStreamRunner;

class CTopology
{
public:
    CTopology();
    ~CTopology();

    bool SetBolt(
        const char* pName,
        tTopologyID id,
        tTopologyID upStreamID,
        CBoltChain* pBoltChain);
    bool SetSpout(
        const char* pName,
        tTopologyID id,
        ISpout& spout,
        CBoltChain* pBoltChain);

    bool DeclareBolt(const char* pName, tTopologyID id, tTopologyID upStreamID, ...);
    bool DeclareSpout(const char* pName, tTopologyID id, ISpout& spout, ...);

    CBoltRunner* GetBoltRunner(tTopologyID id);

    bool Start();
    bool Stop();
    void WaitStopped();

private:
    bool CheckValidity();

private:
    bool m_bRunning;
    CCriticalSection m_CS;
    CCondition m_Cond;
    map<tTopologyID, CBoltRunner*> m_BoltRunners;
    map<tTopologyID, CSpoutRunner*> m_SpoutRunners;
    map<tTopologyID, CStreamRunner*> m_AllRunners;
};

#endif