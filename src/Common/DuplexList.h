/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_DULEX_LIST_H__
#define __COMMON_DULEX_LIST_H__

#include "Common/Typedefs.h"
#include "Memory/Memory.h"
#include "IntrusiveDuList.h"
#include "Tracker/Trace.h"

class CDuplexList
{
public:
    class Iterator
    {
    public:
        Iterator(CIntrusiveDuList::DuListNode* pNode = NULL) : m_pCurrent(pNode) {}
        Iterator(const Iterator& iter) : m_pCurrent(iter.m_pCurrent) {}
        ~Iterator() {}

    public:
        Iterator& operator++()
        {
            ASSERT(m_pCurrent);
            m_pCurrent = m_pCurrent->GetNext();
            return *this;
        }

        Iterator& operator--()
        {
            ASSERT(m_pCurrent);
            m_pCurrent = m_pCurrent->GetPrev();
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
        CIntrusiveDuList::DuListNode* m_pCurrent;
        friend class CDuplexList;
    };

public:
    CDuplexList(CMemory* pAllocator = CMemory::GetDefaultMemory()) :
        m_DuList(),
        m_pMemoryAllocator(pAllocator),
        m_CellSize(sizeof(void*)),
        m_bAggregated(false)
    {
        ASSERT(pAllocator);
        ASSERT(pAllocator->MaxChunkSize() >= sizeof(CIntrusiveDuList::DuListNode) + m_CellSize);
    }

    CDuplexList(size_t cellSize, CMemory* pAllocator = CMemory::GetDefaultMemory()) :
        m_DuList(),
        m_pMemoryAllocator(pAllocator),
        m_CellSize(cellSize),
        m_bAggregated(true)
    {
        ASSERT(cellSize > 0);
        ASSERT(pAllocator);
        ASSERT(pAllocator->MaxChunkSize() >= sizeof(CIntrusiveDuList::DuListNode) + m_CellSize);
    }

    /**
     * @warning This will cause the rhs object clear.
     */
    CDuplexList(CDuplexList& rhs) :
        m_DuList(rhs.m_DuList),
        m_pMemoryAllocator(rhs.m_pMemoryAllocator),
        m_CellSize(rhs.m_CellSize),
        m_bAggregated(rhs.m_bAggregated)
    {
        rhs.Clear();
    }

    ~CDuplexList() { Reset(); }

    size_t Count() const
    {
        return m_DuList.Count();
    }

    void* First()
    {
        CIntrusiveDuList::DuListNode* pNode = m_DuList.First();
        if (pNode && pNode->GetData()) {
            return m_bAggregated ? pNode->GetData() :
                *reinterpret_cast<void**>(pNode->GetData());
        }
        return NULL;
    }

    void* Last()
    {
        CIntrusiveDuList::DuListNode* pNode = m_DuList.Last();
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

    bool PushBack(const void* pData)
    {
        return InsertBefore(End(), pData);
    }

    void PopFront()
    {
        Pop(m_DuList.First());
    }

    void PopAt(Iterator posIter)
    {
        Pop(posIter.m_pCurrent);
    }

    /**
     * @brief Push an item to the front (insert before first position).
     * @param pItem the inserted node item, should not be NULL.
     * @return true if success otherwise false.
     */
    bool PushFront(const void* pItem)
    {
        return InsertBefore(Begin(), pItem);
    }

    /**
     * @brief Append the list (concatenate 2 lists).
     * @param list Specify the appended list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool PushBack(CDuplexList& list, bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Add the list on front (concatenate 2 lists).
     * @param list Specify the appended list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool PushFront(CDuplexList& list, bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Insert list on the specified position (concatenate 2 lists).
     * @param posIter Specify the position
     * @param list Specify the inserted list.
     * @param bTransferOwnershipIfNeeded
     *        Transfer the list node ownership if 2 list share the memory allocator.
     *        If ownership is transferred, the parameter list will be reset.
     * @return true if success, otherwise false
     */
    bool InsertBefore(
        Iterator posIter, CDuplexList& list,
        bool bTransferOwnershipIfNeeded = true);

    /**
     * @brief Insert the node item before the specified position.
     * @param posIter Specify the position by iterator.
     * @param pItem the inserted node item, should not be NULL.
     * @return void
     */
    bool InsertBefore(Iterator posIter, const void* pItem);

    bool DeepCopy(CDuplexList& list);
    void Reset();

    /**
     * @warning Shallow will cause 2 pointer share the same object.
     */
    void ShallowCopy(CDuplexList& list)
    {
        Reset();
        m_DuList = list.m_DuList;
        m_pMemoryAllocator = list.m_pMemoryAllocator;
    }

    void Clear()
    {
        m_DuList.Reset();
    }

    bool IsAggregated() const { return m_bAggregated; }

    CDuplexList::Iterator PositionAt(size_t index)
    {
        return Iterator(m_DuList.PositionAt(index));
    }

    CDuplexList::Iterator Begin()
    {
        return Iterator(m_DuList.First());
    }

    CDuplexList::Iterator End() const
    {
        return Iterator(NULL);
    }

    static size_t ListNodeSize(size_t dataSize = sizeof(void*))
    {
        return sizeof(CIntrusiveDuList::DuListNode) + dataSize;
    }

private:
    void Pop(CIntrusiveDuList::DuListNode* pNode)
    {
        m_DuList.Pop(pNode);
        if (pNode) {
            m_pMemoryAllocator->Free(pNode);
        }
    }

    bool IsCompatiable(CDuplexList& list) const
    {
        return m_CellSize == list.m_CellSize && m_bAggregated == list.m_bAggregated;
    }


private:
    CIntrusiveDuList m_DuList;
    CMemory* m_pMemoryAllocator; // Not owned
    size_t m_CellSize;
    bool m_bAggregated;

    DISALLOW_COPY_CONSTRUCTOR(CDuplexList);
    DISALLOW_ASSIGN_OPERATOR(CDuplexList);
};

#endif