/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ForwardList.h"
#include <new>

bool CForwardList::PushBack(
    CForwardList& list, bool bTransferOwnershipIfNeeded /* = true */)
{
    ASSERT(IsCompatiable(list));

    CIntrusiveFwList* pIntrusiveList = &list.m_FwList;
    CForwardList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_FwList;
    }

    m_FwList.PushBack(*pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CForwardList::InsertAfter(
    Iterator posIter, CForwardList& list,
    bool bTransferOwnershipIfNeeded /* = true */)
{
    ASSERT(IsCompatiable(list));

    CIntrusiveFwList* pIntrusiveList = &list.m_FwList;
    CForwardList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_FwList;
    }
    m_FwList.InsertAfter(posIter.m_pCurrent, *pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CForwardList::PushFront(CForwardList& list, bool bTransferOwnershipIfNeeded /* = true */)
{
    CIntrusiveFwList* pIntrusiveList = &list.m_FwList;
    CForwardList temp(m_pMemoryAllocator);
    if (m_pMemoryAllocator != list.m_pMemoryAllocator ||
        !bTransferOwnershipIfNeeded) {
        // Clone the list
        if (!temp.DeepCopy(list)) {
            return false;
        }
        pIntrusiveList = &temp.m_FwList;
    }
    m_FwList.PushFront(*pIntrusiveList);
    pIntrusiveList->Reset();
    return true;
}

bool CForwardList::InsertAfter(Iterator posIter, void* pItem)
{
    bool bRes = false;
    void* pMem = m_pMemoryAllocator->Malloc(
        sizeof(CIntrusiveFwList::FwListNode) + m_CellSize);
    if (pMem) {
        void* pData = m_bAggregated ? pItem : &pItem;
        CIntrusiveFwList::FwListNode* pNode =
            new (pMem) CIntrusiveFwList::FwListNode(pData, m_CellSize);
        m_FwList.InsertAfter(posIter.m_pCurrent, pNode);
        bRes = true;
    }
    return bRes;
}

bool CForwardList::PushFront(void* pItem)
{
    bool bRes = false;
    void* pMem = m_pMemoryAllocator->Malloc(
        sizeof(CIntrusiveFwList::FwListNode) + m_CellSize);
    if (pMem) {
        void* pData = m_bAggregated ? pItem : &pItem;
        CIntrusiveFwList::FwListNode* pNode =
            new (pMem) CIntrusiveFwList::FwListNode(pData, m_CellSize);
        m_FwList.PushFront(pNode);
        bRes = true;
    }
    return bRes;
}

bool CForwardList::DeepCopy(CForwardList& list)
{
    bool bSuccess = true;
    CForwardList clone(list.m_pMemoryAllocator);
    CIntrusiveFwList::FwListNode* pCur = list.m_FwList.First();
    while (pCur) {
        if (!clone.PushBack(pCur->GetData())) {
            bSuccess = false;
            break;
        }
        pCur = pCur->GetNext();
    }
    if (bSuccess) {
        Reset();
        m_FwList = clone.m_FwList;
        clone.Clear();
    }
    return bSuccess;
}

void CForwardList::Reset()
{
    CIntrusiveFwList::FwListNode* pCur = m_FwList.First();
    while (pCur) {
        CIntrusiveFwList::FwListNode* pNext = pCur->GetNext();
        m_pMemoryAllocator->Free(pCur);
        pCur = pNext;
    }
    Clear();
}
