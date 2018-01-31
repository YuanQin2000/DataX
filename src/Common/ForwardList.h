/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_FORWARD_LIST_H__
#define __COMMON_FORWARD_LIST_H__

#include "Typedefs.h"
#include "IntrusiveFwList.h"
#include "Memory/Memory.h"
#include "Tracker/Trace.h"

class CForwardList
{
public:
    class Iterator
    {
    public:
        Iterator(CIntrusiveFwList::FwListNode* pNode = NULL) : m_pCurrent(pNode) {}
        Iterator(const Iterator& iter) : m_pCurrent(iter.m_pCurrent) {}
        ~Iterator() {}

        Iterator& operator++()
        {
            ASSERT(m_pCurrent);
            m_pCurrent = m_pCurrent->GetNext();
            return *this;
        }

        Iterator& operator=(const Iterator& rhs)
        {
            m_pCurrent = rhs.m_pCurrent;
            return *this;
        }

        bool operator==(const Iterator& rhs)
        {
            return m_pCurrent == rhs.m_pCurrent;
        }

        bool operator!=(const Iterator& rhs)
        {
            return m_pCurrent != rhs.m_pCurrent;
        }

    private:
        CIntrusiveFwList::FwListNode* m_pCurrent;
        friend class CForwardList;
    };

public:
    CForwardList(CMemory* pAllocator = CMemory::GetDefaultMemory()) :
        m_FwList(),
        m_pMemoryAllocator(pAllocator),
        m_CellSize(sizeof(void*)),
        m_bAggregated(false)
    {
        ASSERT(pAllocator);
        ASSERT(pAllocator->MaxChunkSize() >= sizeof(CIntrusiveFwList::FwListNode) + m_CellSize);
    }

    CForwardList(
        size_t cellSize, CMemory* pAllocator = CMemory::GetDefaultMemory()) :
        m_FwList(),
        m_pMemoryAllocator(pAllocator),
        m_CellSize(cellSize),
        m_bAggregated(true)
    {
        ASSERT(cellSize > 0);
        ASSERT(pAllocator);
        ASSERT(pAllocator->MaxChunkSize() >= sizeof(CIntrusiveFwList::FwListNode) + m_CellSize);
    }

    /**
     * @warning This will cause the rhs object clear.
     */
    CForwardList(CForwardList& rhs) :
        m_FwList(rhs.m_FwList),
        m_pMemoryAllocator(rhs.m_pMemoryAllocator),
        m_CellSize(rhs.m_CellSize),
        m_bAggregated(rhs.m_bAggregated)
    {
        rhs.Clear();
    }

    ~CForwardList() { Reset(); }

    size_t Count() const { return m_FwList.Count(); }

    void* First()
    {
        CIntrusiveFwList::FwListNode* pNode = m_FwList.First();
        if (pNode && pNode->GetData()) {
            return m_bAggregated ? pNode->GetData() :
                *reinterpret_cast<void**>(pNode->GetData());
        }
        return NULL;
    }

    void* Last()
    {
        CIntrusiveFwList::FwListNode* pNode = m_FwList.Last();
        if (pNode && pNode->GetData()) {
            return m_bAggregated ? pNode->GetData() :
                *reinterpret_cast<void**>(pNode->GetData());
        }
        return NULL;
    }

    void* DataAt(Iterator iter)
    {
        if (iter.m_pCurrent && iter.m_pCurrent->GetData()) {
            return m_bAggregated ? iter.m_pCurrent->GetData() :
                *reinterpret_cast<void**>(iter.m_pCurrent->GetData());
        }
        return NULL;
    }

    bool PushBack(void* pData)
    {
        return InsertAfter(m_FwList.Last(), pData);
    }

    /**
     * @brief Append the list (concatenate 2 lists).
     * @param list Specify the appended list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool PushBack(CForwardList& list, bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Insert list on the specified position (concatenate 2 lists).
     * @param posIter Specify the position
     * @param list Specify the inserted list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool InsertAfter(
        Iterator posIter, CForwardList& list,
        bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Add the list on front (concatenate 2 lists).
     * @param list Specify the appended list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool PushFront(CForwardList& list, bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Insert the node item after the specified position.
     * @param posIter Specify the position by iterator.
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    bool InsertAfter(Iterator posIter, void* pItem);

    /**
     * @brief Pop the node after specified position.
     * @param posIter Specify the position by iterator.
     */
    void PopAfter(Iterator posIter)
    {
        CIntrusiveFwList::FwListNode* pNode = m_FwList.PopAfter(posIter.m_pCurrent);
        if (pNode) {
            m_pMemoryAllocator->Free(pNode);
        }
    }

    /**
     * @brief Pop the front (first node)
     */
    void PopFront()
    {
        CIntrusiveFwList::FwListNode* pNode = m_FwList.PopFront();
        if (pNode) {
            m_pMemoryAllocator->Free(pNode);
        }
    }

    /**
     * @brief Push an item to the front (insert before first position).
     * @param pItem the inserted node item, should not be NULL.
     * @return true if success, otherwise false.
     */
    bool PushFront(void* pItem);

    bool DeepCopy(CForwardList& list);
    void Reset();

    /**
     * @warning Shallow will cause 2 pointer share the same object.
     */
    void ShallowCopy(CForwardList& list)
    {
        Reset();
        m_FwList = list.m_FwList;
        m_pMemoryAllocator = list.m_pMemoryAllocator;
        m_CellSize = list.m_CellSize;
        m_bAggregated = list.m_bAggregated;
    }

    void Clear()
    {
        m_FwList.Reset();
    }

    bool IsAggregated() const { return m_bAggregated; }

    Iterator PositionAt(size_t index)
    {
        return Iterator(m_FwList.PositionAt(index));
    }

    Iterator Begin()
    {
        return Iterator(m_FwList.First());
    }

    Iterator End() const
    {
        return Iterator(NULL);
    }

    static size_t ListNodeSize(size_t dataSize = sizeof(void*))
    {
        return sizeof(CIntrusiveFwList::FwListNode) + dataSize;
    }

private:
    bool IsCompatiable(CForwardList& list) const
    {
        return m_CellSize == list.m_CellSize && m_bAggregated == list.m_bAggregated;
    }

private:
    CIntrusiveFwList m_FwList;
    CMemory* m_pMemoryAllocator;
    size_t m_CellSize;
    bool m_bAggregated;

    DISALLOW_COPY_CONSTRUCTOR(CForwardList);
    DISALLOW_ASSIGN_OPERATOR(CForwardList);
};

#endif