/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_GRAPH_H__
#define __COMMON_GRAPH_H__

#include "ForwardList.h"

class CMemory;
class CDGNode
{
public:
    CDGNode() : m_ForwardNodes() {}
    CDGNode(CMemory& memAllocator) : m_ForwardNodes(&memAllocator) {}
    virtual ~CDGNode() {}

    bool AddForwardNode(CDGNode* pNode) { return m_ForwardNodes.PushBack(pNode); }
    size_t ForwardCount() { return m_ForwardNodes.Count(); }
    CForwardList& GetForwardNodes() { return m_ForwardNodes; }

private:
    CForwardList m_ForwardNodes;
};

class CDiGraph
{
};

#endif