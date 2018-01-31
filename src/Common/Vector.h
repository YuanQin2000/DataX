/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_VECTOR_H__
#define __COMMON_VECTOR_H__

#include "Typedefs.h"
#include <cstdlib>
#include <cstring>
#include "Tracker/Trace.h"

using std::free;
using std::memcpy;

class CVector
{
public:
    CVector(
        size_t cellSize,
        void* pBuffer,
        size_t bufLen,
        bool bTransferOwnership = false);
    CVector(void* pBuffer, size_t bufLen, bool bTransferOwnership = false);
    CVector(size_t cellSize, size_t expandSize);
    CVector(size_t expandSize);
    ~CVector();

    bool IsAggregated() const { return m_bAggregated; }
    size_t Capacity() const { return m_Capacity; }
    size_t Count() const { return m_Count; }

    void Clear()
    {
        m_pFirst = m_pBuffer;
        m_Count = 0;
    }

    bool PushBack(void* pItem) { return Insert(m_Count, pItem); }
    bool PushFront(void* pItem) { return Insert(0, pItem); }
    void PopBack() { Erase(m_Count - 1); }
    void PopFront() { Erase(0); }

    void* At(size_t posIndex)
    {
        ASSERT(posIndex < m_Count);
        return m_bAggregated ?  PositionAt(posIndex) :
            *reinterpret_cast<void**>(PositionAt(posIndex));
    }

    void Set(size_t posIndex, void* pItem)
    {
        ASSERT(posIndex < m_Count);
        Override(PositionAt(posIndex), pItem);
    }

    bool Insert(size_t posIndex, void* pItem);
    void Erase(size_t posIndex);

private:
    uint8_t* PositionAt(size_t posIndex)
    {
        return m_pFirst + posIndex * m_CellSize;
    }

    void Override(void* pPos, void* pItem)
    {
        memcpy(pPos, m_bAggregated ? pItem : &pItem, m_CellSize);
    }

private:
    uint8_t* m_pBuffer;
    uint8_t* m_pFirst;
    uint32_t m_Capacity;
    uint32_t m_Count;
    const uint32_t m_CellSize;
    const uint32_t m_ExpandCount;
    const bool m_bOwnBuffer;
    const bool m_bAggregated;

    DISALLOW_DEFAULT_CONSTRUCTOR(CVector);
    DISALLOW_COPY_CONSTRUCTOR(CVector);
    DISALLOW_ASSIGN_OPERATOR(CVector);
};

#endif