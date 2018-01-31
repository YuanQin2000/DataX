/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_INTRUSIVE_FWLIST_H__
#define __COMMON_INTRUSIVE_FWLIST_H__

#include "Common/Typedefs.h"
#include <cstring>

using std::memcpy;

///////////////////////////////////////////////////////////////////////////////
//
// CIntrusiveFwList Definition & Implemenation
//
// Instrusive Forward List.
//
///////////////////////////////////////////////////////////////////////////////
class CIntrusiveFwList
{
public:
    struct FwListNode {
        FwListNode* __pNext__;
        uint8_t __Data__[0];

        FwListNode(void* pData, size_t len) : __pNext__(NULL)
        {
            memcpy(__Data__, pData, len);
        }

        FwListNode* GetNext() { return __pNext__; }
        void SetNext(FwListNode* pNode) { __pNext__ = pNode; }
        void* GetData() { return __Data__; }
    };

    CIntrusiveFwList(void* pItem = NULL) :
        m_pFirst(reinterpret_cast<FwListNode*>(pItem)),
        m_pLast(reinterpret_cast<FwListNode*>(pItem)),
        m_Count(pItem == NULL ? 0 : 1) {}
    CIntrusiveFwList(const CIntrusiveFwList& rhs) :
        m_pFirst(rhs.m_pFirst),
        m_pLast(rhs.m_pLast),
        m_Count(rhs.m_Count) {}
    ~CIntrusiveFwList() {}

    size_t Count() const { return m_Count; }
    FwListNode* First() { return m_pFirst; }
    FwListNode* Last()  { return m_pLast; }

    void Reset()
    {
        m_pFirst = NULL;
        m_pLast = NULL;
        m_Count = 0;
    }

    void PushBack(FwListNode* pNode)
    {
        InsertAfter(m_pLast, pNode);
    }

    void PushBack(CIntrusiveFwList& list)
    {
        InsertAfter(m_pLast, list);
    }

    /**
     * @brief Insert the node item after the specified position.
     * @param posItem Specify the position by list node, should not be NULL.
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    void InsertAfter(FwListNode* posItem, FwListNode* pItem);

    /**
     * @brief Insert the list after the specified position.
     * @param posItem Specify the position by list node, should not be NULL.
     * @param list the inserted list.
     * @return void
     */
    void InsertAfter(FwListNode* posItem, CIntrusiveFwList& list);

    /**
     * @brief Pop the node after specified position.
     * @param posItem Specify the position by list node, should not be NULL.
     * @return the popped node.
     */
    FwListNode* PopAfter(FwListNode* posItem);

    /**
     * @brief Pop the front (first node)
     * @return the popped node.
     */
    FwListNode* PopFront();

    /**
     * @brief Push an item to the front (insert before first position).
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    void PushFront(FwListNode* pItem);

    /**
     * @brief Push a DuList to the front (insert before first position).
     * @param list the inserted fwlist.
     * @return void
     */
    void PushFront(CIntrusiveFwList& list);

    FwListNode* PositionAt(size_t index);
    CIntrusiveFwList& operator=(const CIntrusiveFwList& rhs);

private:
    FwListNode* m_pFirst;
    FwListNode* m_pLast;
    size_t m_Count;
};

#endif