/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "IntrusiveDuList.h"

void CIntrusiveDuList::InsertBefore(DuListNode* posItem, DuListNode* pItem)
{
    ASSERT(pItem);
    ASSERT(pItem->GetNext() == NULL);
    ASSERT(pItem->GetPrev() == NULL);

    if (posItem) {
        DuListNode* pOldPrev = posItem->GetPrev();
        if (pOldPrev) {
            pOldPrev->SetNext(pItem);
        }
        pItem->SetPrev(pOldPrev);
        pItem->SetNext(posItem);
        posItem->SetPrev(pItem);
        if (posItem == m_pFirst) {
            m_pFirst = pItem;
        }
    } else {
        // Insert at the end.
        if (m_pLast) {
            m_pLast->SetNext(pItem);
        }
        pItem->SetPrev(m_pLast);
        m_pLast = pItem;
        if (m_pFirst == NULL) {
            m_pFirst = pItem;
        }
    }
    ++m_Count;
}

void CIntrusiveDuList::InsertBefore(DuListNode* posItem, CIntrusiveDuList& list)
{
    size_t listCount = list.Count();
    if (listCount == 0) {
        return;
    }

    DuListNode* pListFirst = list.First();
    DuListNode* pListLast = list.Last();

    ASSERT(pListFirst);
    ASSERT(pListFirst->GetPrev() == NULL);
    ASSERT(pListLast);
    ASSERT(pListLast->GetNext() == NULL);

    if (posItem) {
        DuListNode* pOldPrev = posItem->GetPrev();
        if (pOldPrev) {
            pOldPrev->SetNext(pListFirst);
        }
        pListFirst->SetPrev(pOldPrev);
        posItem->SetPrev(pListLast);
        pListLast->SetNext(posItem);
        if (m_pFirst == posItem) {
            m_pFirst = pListFirst;
        }
    } else {
        // Insert at the end
        if (m_pLast) {
            m_pLast->SetNext(pListFirst);
        }
        pListFirst->SetPrev(m_pLast);
        m_pLast = pListLast;
        if (m_pFirst == NULL) {
            m_pFirst = pListFirst;
        }
    }
    m_Count += listCount;
}

void CIntrusiveDuList::Pop(DuListNode* pItem)
{
    ASSERT(pItem);

    DuListNode* pOldPrev = pItem->GetPrev();
    DuListNode* pOldNext = pItem->GetNext();
    if (pOldPrev) {
        pOldPrev->SetNext(pOldNext);
    } else {
        // Pop the first.
        m_pFirst = pOldNext;
    }
    if (pOldNext) {
        pOldNext->SetPrev(pOldPrev);
    } else {
        // Pop the last
        m_pLast = pOldPrev;
    }
    pItem->SetPrev(NULL);
    pItem->SetNext(NULL);
    --m_Count;
}

CIntrusiveDuList::DuListNode* CIntrusiveDuList::PositionAt(size_t index) const
{
    ASSERT(index <= m_Count, "index: %d, count: %d\n", index, m_Count);

    DuListNode* pPos = m_pFirst;
    size_t i = 0;
    while (i != index && pPos != NULL) {
        pPos = pPos->GetNext();
        ++i;
    }
    return pPos;
}

CIntrusiveDuList& CIntrusiveDuList::operator=(const CIntrusiveDuList& rhs)
{
    Reset();
    m_pFirst = rhs.m_pFirst;
    m_pLast = rhs.m_pLast;
    m_Count = rhs.m_Count;
    return *this;
}
