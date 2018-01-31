/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DuplexList.h"
#include <new>

bool CDuplexList::PushBack(CDuplexList& list, bool bTransferOwnershipIfNeeded /* = true */)
{
    ASSERT(IsCompatiable(list));

    CIntrusiveDuList* pIntrusiveList = &list.m_DuList;
    CDuplexList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_DuList;
    }
    m_DuList.PushBack(*pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CDuplexList::PushFront(CDuplexList& list, bool bTransferOwnershipIfNeeded /* = true */)
{
    ASSERT(IsCompatiable(list));

    CIntrusiveDuList* pIntrusiveList = &list.m_DuList;
    CDuplexList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_DuList;
    }
    m_DuList.PushFront(*pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CDuplexList::InsertBefore(
    Iterator posIter, CDuplexList& list,
    bool bTransferOwnershipIfNeeded /* = true */)
{
    ASSERT(IsCompatiable(list));

    CIntrusiveDuList* pIntrusiveList = &list.m_DuList;
    CDuplexList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_DuList;
    }
    m_DuList.InsertBefore(posIter.m_pCurrent, *pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CDuplexList::InsertBefore(Iterator posIter, const void* pItem)
{
    bool bRes = false;
    void* pMem = m_pMemoryAllocator->Malloc(
        sizeof(CIntrusiveDuList::DuListNode) + m_CellSize);
    if (pMem) {
        const void* pData = m_bAggregated ? pItem : &pItem;
        CIntrusiveDuList::DuListNode* pNode =
            new (pMem) CIntrusiveDuList::DuListNode(pData, m_CellSize);
        m_DuList.InsertBefore(posIter.m_pCurrent, pNode);
        bRes = true;
    }
    return bRes;
}

bool CDuplexList::DeepCopy(CDuplexList& list)
{
    bool bSuccess = true;
    CDuplexList clone(m_pMemoryAllocator);
    CIntrusiveDuList::DuListNode* pCur = list.m_DuList.First();
    while (pCur) {
        if (!clone.PushBack(pCur->GetData())) {
            bSuccess = false;
            break;
        }
        pCur = pCur->GetNext();
    }
    if (bSuccess) {
        Reset();
        m_DuList = clone.m_DuList;
        clone.Clear();
    }
    return bSuccess;
}

void CDuplexList::Reset()
{
    CIntrusiveDuList::DuListNode* pCur = m_DuList.First();
    while (pCur) {
        CIntrusiveDuList::DuListNode* pNext = pCur->GetNext();
        m_pMemoryAllocator->Free(pCur);
        pCur = pNext;
    }
    m_DuList.Reset();
}
