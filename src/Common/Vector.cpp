/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Vector.h"
#include <cstring>

using std::memmove;
using std::malloc;
using std::realloc;

CVector::CVector(
    size_t cellSize,
    void* pBuffer,
    size_t bufLen,
    bool bTransferOwnership /* = false */) :
    m_pBuffer(reinterpret_cast<uint8_t*>(pBuffer)),
    m_pFirst(reinterpret_cast<uint8_t*>(pBuffer)),
    m_Capacity(bufLen / cellSize),
    m_Count(0),
    m_CellSize(cellSize),
    m_ExpandCount(0),
    m_bOwnBuffer(bTransferOwnership),
    m_bAggregated(true)
{
    ASSERT(cellSize > 0);
    ASSERT(pBuffer);
    ASSERT(m_Capacity > 0);
}

CVector::CVector(
    void* pBuffer,
    size_t bufLen,
    bool bTransferOwnership /* = false */) :
    m_pBuffer(reinterpret_cast<uint8_t*>(pBuffer)),
    m_pFirst(reinterpret_cast<uint8_t*>(pBuffer)),
    m_Capacity(bufLen / sizeof(void*)),
    m_Count(0),
    m_CellSize(sizeof(void*)),
    m_ExpandCount(0),
    m_bOwnBuffer(bTransferOwnership),
    m_bAggregated(false)
{
    ASSERT(pBuffer);
    ASSERT(bufLen > 0);
    ASSERT(m_Capacity > 0);
}

CVector::CVector(size_t cellSize, size_t expandSize) :
    m_pBuffer(NULL),
    m_pFirst(NULL),
    m_Capacity(0),
    m_Count(0),
    m_CellSize(cellSize),
    m_ExpandCount(expandSize),
    m_bOwnBuffer(true),
    m_bAggregated(true)
{
    ASSERT(cellSize > 0);
    ASSERT(expandSize > 0);
}

CVector::CVector(size_t expandSize) :
    m_pBuffer(NULL),
    m_pFirst(NULL),
    m_Capacity(0),
    m_Count(0),
    m_CellSize(sizeof(void*)),
    m_ExpandCount(expandSize),
    m_bOwnBuffer(true),
    m_bAggregated(false)
{
    ASSERT(expandSize > 0);
}

CVector::~CVector()
{
    if (m_bOwnBuffer && m_pBuffer) {
        free(m_pBuffer);
    }
}

bool CVector::Insert(size_t posIndex, void* pItem)
{
    ASSERT(posIndex <= m_Count);

    uint8_t* pInsertPos = NULL;
    size_t frontPartSize = (posIndex - 0) * m_CellSize;
    size_t backPartSize = (m_Count - posIndex) * m_CellSize;

    if (m_Count < m_Capacity) {
        bool bMoveRightward;
        if (m_pFirst == m_pBuffer) {
            bMoveRightward = true;
        } else if (PositionAt(m_Count) == m_pBuffer + m_Capacity * m_CellSize) {
            bMoveRightward = false;
        } else {
            bMoveRightward = frontPartSize > backPartSize;
        }

        if (bMoveRightward) {
            pInsertPos = PositionAt(posIndex);
            if (backPartSize > 0) {
                memmove(pInsertPos + m_CellSize, pInsertPos, backPartSize);
            }
        } else {
            pInsertPos = PositionAt(posIndex - 1);
            if (frontPartSize > 0) {
                memmove(m_pFirst - m_CellSize, m_pFirst, frontPartSize);
            }
            m_pFirst -= m_CellSize;
        }
    } else {
        if (m_ExpandCount == 0) {
            // Can not expand.
            return false;
        }

        size_t newSize = (m_Capacity + m_ExpandCount) * m_CellSize;
        uint8_t* pTmp = reinterpret_cast<uint8_t*>(malloc(newSize));
        if (pTmp == NULL) {
            OUTPUT_WARNING_TRACE("malloc failed!\n");
            return false;
        }

        pInsertPos = pTmp + posIndex * m_CellSize;
        if (frontPartSize > 0) {
            memcpy(pTmp, m_pFirst, frontPartSize);
        }
        if (backPartSize > 0) {
            memcpy(pInsertPos + m_CellSize, PositionAt(posIndex), backPartSize);
        }

        if (m_pBuffer) {
            free(m_pBuffer);
        }
        m_pBuffer = pTmp;
        m_pFirst = pTmp;
        m_Capacity += m_ExpandCount;
    }

    void* pInsertItem = m_bAggregated ? pItem : &pItem;
    memcpy(pInsertPos, pInsertItem, m_CellSize);
    ++m_Count;
    return true;
}

void CVector::Erase(size_t posIndex)
{
    ASSERT(posIndex < m_Count);

    uint8_t* pErasePos = PositionAt(posIndex);
    size_t frontPartSize = (posIndex - 0) * m_CellSize;
    size_t backPartSize = (m_Count - posIndex - 1) * m_CellSize;

    if (frontPartSize < backPartSize) {
        if (frontPartSize > 0) {
            memmove(m_pFirst + m_CellSize, m_pFirst, frontPartSize);
        }
        m_pFirst += m_CellSize;
    } else {
        if (backPartSize > 0) {
            memmove(pErasePos, pErasePos + m_CellSize, backPartSize);
        }
    }

    --m_Count;
    if (m_Count == 0) {
        Clear();
    }
}
