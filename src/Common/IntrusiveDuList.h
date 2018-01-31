/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_INTRUSIVE_DULIST_H__
#define __COMMON_INTRUSIVE_DULIST_H__

#include "Common/Typedefs.h"
#include <stdio.h>
#include <cstring>
#include "Tracker/Trace.h"

using std::memcpy;

///////////////////////////////////////////////////////////////////////////////
//
// CIntrusiveDuList Definition & Implemenation
//
// Instrusive Duplex List.
//
///////////////////////////////////////////////////////////////////////////////
class CIntrusiveDuList
{
public:
    struct DuListNode {
        DuListNode* __pNext__;
        DuListNode* __pPrev__;
        uint8_t __Data__[0];

        DuListNode(const void* pData, size_t len) : __pNext__(NULL), __pPrev__(NULL)
        {
            memcpy(__Data__, pData, len);
        }

        DuListNode* GetNext() { return __pNext__; }
        DuListNode* GetPrev() { return __pPrev__; }
        void SetNext(DuListNode* pNode) { __pNext__ = pNode; }
        void SetPrev(DuListNode* pNode) { __pPrev__ = pNode; }
        void* GetData() { return __Data__; }
    };

public:
    CIntrusiveDuList(DuListNode* pItem = NULL) :
        m_pFirst(pItem),
        m_pLast(pItem),
        m_Count(0) {}
    CIntrusiveDuList(const CIntrusiveDuList& rhs) :
        m_pFirst(rhs.m_pFirst),
        m_pLast(rhs.m_pLast),
        m_Count(rhs.m_Count) {}
    ~CIntrusiveDuList() {}

    size_t Count() const { return m_Count; }
    DuListNode* First() { return m_pFirst; }
    DuListNode* Last()  { return m_pLast; }

    void Reset()
    {
        m_pFirst = NULL;
        m_pLast = NULL;
        m_Count = 0;
    }

    /**
     * @brief Push to the end (insert after the last node)
     * @param pNode the inserted node item, should not be NULL.
     * @return the popped node.
     */
    void PushBack(DuListNode* pNode)
    {;
        InsertBefore(NULL, pNode);
    }

    /**
     * @brief Push to the end (insert after the last node)
     * @param list the inserted list should not be NULL.
     * @return the popped node.
     */
    void PushBack(CIntrusiveDuList& list)
    {
        InsertBefore(NULL, list);
    }

    /**
     * @brief Pop the front (first node)
     * @return the popped node.
     */
    DuListNode* PopFront()
    {
        Pop(m_pFirst);
        return m_pFirst;
    }

    /**
     * @brief Push an item to the front (insert before first position).
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    void PushFront(DuListNode* pItem)
    {
        InsertBefore(m_pFirst, pItem);
    }

    /**
     * @brief Push a DuList to the front (insert before first position).
     * @param list the inserted fwlist.
     * @return void
     */
    void PushFront(CIntrusiveDuList& list)
    {
        InsertBefore(m_pFirst, list);
    }

    /**
     * @brief Insert the node item before the specified position.
     * @param pPosItem Specify the position by node item.
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    void InsertBefore(DuListNode* pPosItem, DuListNode* pItem);

    /**
     * @brief Insert a DuList after the specified position.
     * @param posItem Specify the position by node item.
     * @param list the inserted dulist.
     * @return void
     */
    void InsertBefore(DuListNode* pPosItem, CIntrusiveDuList& list);

    /**
     * @brief Pop the specified node.
     * @param posIter Specify the position by iterator.
     * @return the popped node.
     */
    void Pop(DuListNode* pPosItem);

    DuListNode* PositionAt(size_t index) const;
    DuListNode* First() const { return m_pFirst; }

    CIntrusiveDuList& operator=(const CIntrusiveDuList& rhs);

private:
    DuListNode* m_pFirst;
    DuListNode* m_pLast;
    size_t m_Count;
};

#endif
