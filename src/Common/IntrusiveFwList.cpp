/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "IntrusiveFwList.h"
#include "Memory/LazyBuffer.h"
#include "Tracker/Trace.h"
#include <cstdlib>

using std::malloc;
using std::free;

void CIntrusiveFwList::InsertAfter(FwListNode* posItem, FwListNode* pItem)
{
    ASSERT(pItem);
    ASSERT(pItem->GetNext() == NULL);

    if (m_Count > 0) {
        ASSERT(posItem);
        pItem->SetNext(posItem->GetNext());
        posItem->SetNext(pItem);
        if (posItem == m_pLast) {
            m_pLast = pItem;
        }
    } else {
        ASSERT(posItem == NULL);
        m_pFirst = m_pLast = pItem;
    }
    ++m_Count;
}

void CIntrusiveFwList::InsertAfter(FwListNode* posItem, CIntrusiveFwList& list)
{
    size_t listCount = list.Count();
    if (listCount == 0) {
        return;
    }

    FwListNode* pListFirst = list.First();
    FwListNode* pListLast = list.Last();

    ASSERT(pListFirst);
    ASSERT(pListLast);
    ASSERT(pListLast->GetNext() == NULL);

    if (m_Count > 0) {
        ASSERT(posItem);
        pListLast->SetNext(posItem->GetNext());
        posItem->SetNext(pListFirst);
        m_pLast = pListLast;
    } else {
        ASSERT(posItem == NULL);
        m_pFirst = pListFirst;
        m_pLast = pListLast;
    }
    m_Count += listCount;
}

CIntrusiveFwList::FwListNode* CIntrusiveFwList::PopAfter(FwListNode* posItem)
{
    FwListNode* pObj = NULL;
    if (posItem) {
        pObj = posItem->GetNext();
        if (pObj) {
            posItem->SetNext(pObj->GetNext());
            if (pObj == m_pLast) {
                m_pLast = posItem;
            }
            --m_Count;
            pObj->SetNext(NULL);
        }
    }
    return pObj;
}

CIntrusiveFwList::FwListNode* CIntrusiveFwList::PopFront()
{
    FwListNode* pObj = m_pFirst;
    if (m_pFirst) {
        m_pFirst = m_pFirst->GetNext();
        pObj->SetNext(NULL);
        --m_Count;
        if (m_pFirst == NULL) {
            ASSERT(m_Count == 0);
            ASSERT(m_pLast == pObj);
            m_pLast = NULL;
        }
    }
    return pObj;
}

void CIntrusiveFwList::PushFront(FwListNode* pItem)
{
    ASSERT(pItem->GetNext() == NULL);

    pItem->SetNext(m_pFirst);
    m_pFirst = pItem;
    if (m_pLast == NULL) {
        m_pLast = pItem;
    }
    ++m_Count;
}

void CIntrusiveFwList::PushFront(CIntrusiveFwList& list)
{
    size_t listCount = list.Count();
    if (listCount == 0) {
        return;
    }
    FwListNode* pListFirst = list.First();
    FwListNode* pListLast = list.Last();

    ASSERT(pListFirst);
    ASSERT(pListLast);
    ASSERT(pListLast->GetNext() == NULL);

    pListLast->SetNext(m_pFirst);
    m_pFirst = pListFirst;
    if (m_pLast == NULL) {
        m_pLast = pListLast;
    }
    m_Count += listCount;
}

CIntrusiveFwList::FwListNode* CIntrusiveFwList::PositionAt(size_t index)
{
    ASSERT(index <= m_Count, "index: %d, count: %d\n", index, m_Count);

    FwListNode* pPos = m_pFirst;
    size_t i = 0;
    while (i != index && pPos != NULL) {
        pPos = pPos->GetNext();
        ++i;
    }
    return pPos;
}

CIntrusiveFwList& CIntrusiveFwList::operator=(const CIntrusiveFwList& rhs)
{
    Reset();
    m_pFirst = rhs.m_pFirst;
    m_pLast = rhs.m_pLast;
    m_Count = rhs.m_Count;
    return *this;
}
