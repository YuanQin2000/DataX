/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_TREE_H__
#define __COMMON_TREE_H__

#include <new>
#include <map>
#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Memory/MemoryPool.h"
#include "Tracker/Trace.h"

#ifdef __DEBUG__
#include <stdio.h>
#endif


using std::pair;

template <typename T>
class CTree
{
public:
    class CNode
    {
    public:
        CNode(T* pElem) :
            m_pElement(pElem),
            m_pParent(NULL),
            m_pFirstChild(NULL),
            m_pLastChild(NULL),
            m_pSlibings(NULL),
            m_Depth(0) {}
        ~CNode() {}

        T* GetElement()     const { return m_pElement;    }
        CNode* GetParent()  const { return m_pParent;     }
        CNode* GetChild()   const { return m_pFirstChild; }
        CNode* GetSlibing() const { return m_pSlibings;   }
        CNode* GetRoot()    const;

        void AddChild(CNode* pNode);
        CNode* Find(const T& elem, size_t depth = 0);
        size_t Count(size_t depth = 0) const;

#ifdef __DEBUG__
        void Dump();
#endif

    private:
        T*     m_pElement;
        CNode* m_pParent;
        CNode* m_pFirstChild;
        CNode* m_pLastChild;
        CNode* m_pSlibings;
        size_t m_Depth;

        DISALLOW_DEFAULT_CONSTRUCTOR(CNode);
        DISALLOW_COPY_CONSTRUCTOR(CNode);
        DISALLOW_ASSIGN_OPERATOR(CNode);
    };

    class CDFSTraverser
    {
    public:
        CDFSTraverser(CNode* pStart) :
            m_pStart(pStart),
            m_pCurrent(pStart)
        {
            ASSERT(pStart);
            m_Depth = 0;
        }

        ~CDFSTraverser() {}

        CNode* GetNext();
        size_t GetDepth() const { return m_Depth; }
    private:
        CNode* m_pStart;
        CNode* m_pCurrent;
        size_t m_Depth;
    };

public:
    CTree() : m_pRoot(NULL), m_MemoryPool(sizeof(CNode)) {}
    ~CTree() {}

    CNode* GetRoot() const      { return m_pRoot; }
    CNode* CreateRoot(T* pElem) { return (m_pRoot = AddChild(NULL, pElem)); }
    CNode* AddChild(CNode* pParent, T* pElem);
    void RemoveNode(CNode* pNode);

#ifdef __DEBUG__
    void Dump() const;
#endif

private:
    CNode* m_pRoot;
    CMemoryPool m_MemoryPool;
};

template <typename T>
typename CTree<T>::CNode* CTree<T>::CNode::GetRoot() const
{
    CNode* pNode = this;
    while (pNode->m_pParent) {
        pNode = pNode->m_pParent;
    }
    return pNode;
}

template <typename T>
void CTree<T>::CNode::AddChild(CNode* pNode)
{
    pNode->m_pParent = this;
    if (!m_pFirstChild) {
        m_pFirstChild = pNode;
    }
    if (m_pLastChild) {
        m_pLastChild->m_pSlibings = pNode;
    }
    m_pLastChild = pNode;
    pNode->m_Depth = m_Depth + 1;
}

template <typename T>
typename CTree<T>::CNode* CTree<T>::CNode::Find(const T& elem, size_t depth /* = 0 */)
{
    CTree<T>::CDFSTraverser traverse(this);
    CNode* pNode = this;
    do {
        T* pElem = pNode->GetElement();
        if (*pElem == elem && depth == traverse.GetDepth()) {
            break;
        }
        pNode = traverse.GetNext();
    } while (pNode);
    return pNode;
}

template <typename T>
size_t CTree<T>::CNode::Count(size_t depth /* = 0 */) const
{
    size_t count = 1;   // including this
    CTree<T>::CDFSTraverser traverse(this);
    CNode* pNode = traverse.GetNext();
    while (pNode) {
        if (depth == 0 || depth == traverse.GetDepth()) {
            ++count;
        }
        pNode = traverse.GetNext();
    }
    return count;
}

#ifdef __DEBUG__
template <typename T>
void CTree<T>::CNode::Dump()
{
    CTree<T>::CDFSTraverser traverse(this);
    CNode* pNode = this;

    do {
        T* pElem = pNode->GetElement();
        for (size_t i = 0; i < traverse.GetDepth(); ++i) {
            putchar('\t');
        }
        pElem->Dump();
        pNode = traverse.GetNext();
    } while (pNode);
}
#endif

template <typename T>
typename CTree<T>::CNode* CTree<T>::CDFSTraverser::GetNext()
{
    CNode* pNode = m_pCurrent->GetChild();
    if (pNode) {
        m_pCurrent = pNode;
        ++m_Depth;
        return pNode;
    }
    while (m_pCurrent != m_pStart) {
        pNode = m_pCurrent->GetSlibing();
        if (pNode) {
            m_pCurrent = pNode;
            break;
        }
        m_pCurrent = m_pCurrent->GetParent();
        --m_Depth;
    }
    return pNode;
}

template <typename T>
typename CTree<T>::CNode* CTree<T>::AddChild(CNode* pParent, T* pElem)
{
    CNode* pNode = NULL;
    void* pMemory = reinterpret_cast<CNode*>(m_MemoryPool.Malloc(sizeof(CNode)));
    if (pMemory) {
        pNode = new (pMemory) CNode(pElem);
        if (pParent) {
            pParent->AddChild(pNode);
        }
    }
    return pNode;
}

template <typename T>
void CTree<T>::RemoveNode(CNode* pNode)
{
    ASSERT(pNode);

    ASSERT(false);  // TODO: Implement
}

#ifdef __DEBUG__
template <typename T>
void CTree<T>::Dump() const
{
    if (m_pRoot) {
        m_pRoot->Dump();
    }
}
#endif

#endif