/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Topology.h"
#include "Bolt.h"
#include "BoltChain.h"
#include "BoltRunner.h"
#include "SpoutRunner.h"
#include "StreamRunner.h"
#include "Common/Macros.h"
#include "Tracker/Trace.h"
#include <utility>
#include <stdarg.h>
#include <stdio.h>

using std::pair;

CTopology::CTopology() :
    m_bRunning(false),
    m_CS(),
    m_Cond(),
    m_BoltRunners(),
    m_SpoutRunners(),
    m_AllRunners()
{
}

CTopology::~CTopology()
{
    Stop();
}

bool CTopology::SetBolt(
    const char* pName,
    tTopologyID id,
    tTopologyID upStreamID,
    CBoltChain* pBoltChain)
{
    ASSERT(pBoltChain);

    CSectionLock lock(m_CS);
    if (m_bRunning) {
        OUTPUT_ERROR_TRACE("Topology is running!!\n");
        ASSERT(false);
        return false;
    }

    // Find if the ID is in use.
    map<tTopologyID, CStreamRunner*>::iterator findIter = m_AllRunners.find(id);
    if (findIter != m_AllRunners.end()) {
        // Found.
        OUTPUT_ERROR_TRACE("The ID has been existed: %d\n", id);
        return false;
    }

    // Find the upstream node.
    CStreamRunner* pUpStream = NULL;
    findIter = m_AllRunners.find(upStreamID);
    if (findIter != m_AllRunners.end()) {
        pUpStream = findIter->second;
    }
    if (pUpStream == NULL) {
        OUTPUT_ERROR_TRACE("The upstream ID has not been registered: %d\n", upStreamID);
        return NULL;
    }

    CBoltRunner* pRunner = new CBoltRunner(pName, pBoltChain);
    if (pRunner == NULL) {
        OUTPUT_WARNING_TRACE("Can not create bolt runner object for %d.\n", id);
        return false;
    }
    if (!pUpStream->AddForwardNode(pRunner)) {
        OUTPUT_WARNING_TRACE("Can not follow the upstream object.\n");
        delete pRunner;
        return false;
    }

    pair<map<tTopologyID, CBoltRunner*>::const_iterator, bool> ret = 
        m_BoltRunners.insert(map<tTopologyID, CBoltRunner*>::value_type(id, pRunner));
    ASSERT(ret.second);
    pair<map<tTopologyID, CStreamRunner*>::const_iterator, bool> ret1 = 
        m_AllRunners.insert(map<tTopologyID, CStreamRunner*>::value_type(id, pRunner));
    ASSERT(ret1.second);
    return true;
}

bool CTopology::SetSpout(
    const char* pName,
    tTopologyID id,
    ISpout& spout,
    CBoltChain* pBoltChain)
{
    CSectionLock lock(m_CS);
    if (m_bRunning) {
        OUTPUT_ERROR_TRACE("Topology is running!!\n");
        ASSERT(false);
        return false;
    }

    // Find if the ID is in use.
    map<tTopologyID, CStreamRunner*>::iterator findIter = m_AllRunners.find(id);
    if (findIter != m_AllRunners.end()) {
        // Found.
        OUTPUT_ERROR_TRACE("The ID has been existed: %d\n", id);
        return false;
    }

    CSpoutRunner* pRunner = new CSpoutRunner(pName, spout, pBoltChain);
    if (pRunner == NULL) {
        OUTPUT_WARNING_TRACE("Can not create spout runner object for %d.\n", id);
        return false;
    }

    pair<map<tTopologyID, CSpoutRunner*>::const_iterator, bool> ret = 
        m_SpoutRunners.insert(map<tTopologyID, CSpoutRunner*>::value_type(id, pRunner));
    ASSERT(ret.second);
    pair<map<tTopologyID, CStreamRunner*>::const_iterator, bool> ret1 = 
        m_AllRunners.insert(map<tTopologyID, CStreamRunner*>::value_type(id, pRunner));
    ASSERT(ret1.second);
    return true;
}

bool CTopology::DeclareBolt(const char* pName, tTopologyID id, tTopologyID upStreamID, ...)
{
    va_list args;
    va_start(args, upStreamID);
    CBoltChain* pBoltChain = CBoltChain::CreateInstance(args);
    va_end(args);

    bool bRes = false;
    if (pBoltChain) {
        bRes = SetBolt(pName, id, upStreamID, pBoltChain);
        if (!bRes) {
            delete pBoltChain;
            pBoltChain = NULL;
        }
    }
    return bRes;
}

bool CTopology::DeclareSpout(const char* pName, tTopologyID id, ISpout& spout, ...)
{
    va_list args;
    va_start(args, spout);
    CBoltChain* pBoltChain = CBoltChain::CreateInstance(args);
    va_end(args);

    bool bRes = SetSpout(pName, id, spout, pBoltChain);
    if (!bRes && pBoltChain) {
        delete pBoltChain;
        pBoltChain = NULL;
    }
    return bRes;
}

CBoltRunner* CTopology::GetBoltRunner(tTopologyID id)
{
    CSectionLock lock(m_CS);
    if (!m_bRunning) {
        OUTPUT_WARNING_TRACE("Topology is in setup.\n");
        return NULL;
    }

    // Find if the ID is in bolt runner.
    map<tTopologyID, CBoltRunner*>::iterator findIter = m_BoltRunners.find(id);
    if (findIter == m_BoltRunners.end()) {
        // Not Found.
        m_CS.Unlock();
        OUTPUT_ERROR_TRACE("The ID has not been registered as a Bolt: %d\n", id);
        return NULL;
    }
    return findIter->second;
}

bool CTopology::Start()
{
    CSectionLock lock(m_CS);
    if (m_bRunning) {
        return true;
    }
    if (!CheckValidity()) {
        OUTPUT_ERROR_TRACE("Topology is not valid, can not start streaming.\n");
        ASSERT(false);
        return false;
    }

    bool bRes = true;
    map<tTopologyID, CBoltRunner*>::iterator boltIter = m_BoltRunners.begin();
    map<tTopologyID, CBoltRunner*>::iterator boltIterEnd = m_BoltRunners.end();
    while (boltIter != boltIterEnd) {
        if (!boltIter->second->Start()) {
            bRes = false;
            break;
        }
        ++boltIter;
    }
    if (!bRes) {
        OUTPUT_WARNING_TRACE("Running bolt failed.\n");
        return false;
    }

    map<tTopologyID, CSpoutRunner*>::iterator spoutIter = m_SpoutRunners.begin();
    map<tTopologyID, CSpoutRunner*>::iterator spoutIterEnd = m_SpoutRunners.end();
    while (spoutIter != spoutIterEnd) {
        if (!spoutIter->second->Start()) {
            bRes = false;
            break;
        }
        ++spoutIter;
    }
    if (!bRes) {
        OUTPUT_WARNING_TRACE("Running spout failed.\n");
        return false;
    }
    m_bRunning = true;
    return true;
}

bool CTopology::Stop()
{
    CSectionLock lock(m_CS);

    if (!m_bRunning) {
        return true;
    }

    bool bRes = true;
    map<tTopologyID, CSpoutRunner*>::iterator spoutIter = m_SpoutRunners.begin();
    map<tTopologyID, CSpoutRunner*>::iterator spoutIterEnd = m_SpoutRunners.end();
    while (spoutIter != spoutIterEnd) {
        if (!spoutIter->second->Stop()) {
            bRes = false;
            break;
        }
        ++spoutIter;
    }
    if (!bRes) {
        OUTPUT_WARNING_TRACE("Stop spout failed.\n");
        return false;
    }

    map<tTopologyID, CBoltRunner*>::iterator boltIter = m_BoltRunners.begin();
    map<tTopologyID, CBoltRunner*>::iterator boltIterEnd = m_BoltRunners.end();
    while (boltIter != boltIterEnd) {
        if (!boltIter->second->Stop()) {
            bRes = false;
            break;
        }
        ++boltIter;
    }
    if (!bRes) {
        OUTPUT_WARNING_TRACE("Stop bolt failed.\n");
        return false;
    }

    m_bRunning = false;
    m_Cond.Signal(&m_CS);
    return true;
}

void CTopology::WaitStopped()
{
    CSectionLock lock(m_CS);
    if (m_bRunning) {
        m_Cond.Wait(&m_CS);
        ASSERT(!m_bRunning);
    }
}

bool CTopology::CheckValidity()
{
    // TODO: impl
    return true;
}
