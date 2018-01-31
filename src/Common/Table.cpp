/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Table.h"
#include <cstring>

using std::memcpy;
using std::realloc;

void* CTable::At(tIndex idx)
{
    void* pRes = NULL;
    if (idx < m_Capacity) {
        BlockHeader* pHdr = reinterpret_cast<BlockHeader*>(m_pBuffer + idx * m_CellSize);
        if (!TEST_FLAG(pHdr->Flags, FREE_BLOCK_FLAG)) {
            pRes = reinterpret_cast<uint8_t*>(pHdr) + sizeof(BlockHeader);
        }
    }
    return pRes;
}

void CTable::Set(tIndex idx, void* pData)
{
    ASSERT(pData);

    if (idx < m_Capacity) {
        BlockHeader* pHdr = reinterpret_cast<BlockHeader*>(m_pBuffer + idx * m_CellSize);
        if (!TEST_FLAG(pHdr->Flags, FREE_BLOCK_FLAG)) {
            uint8_t* pSlot = reinterpret_cast<uint8_t*>(pHdr) + sizeof(BlockHeader);
            void* pSetData = m_bAggregated ? pData : &pData;
            memcpy(pSlot, pSetData, m_CellSize - sizeof(BlockHeader));
            return;
        }
    }
    ASSERT(false);
}

void* CTable::Allocate(tIndex* pOutIndex)
{
    ASSERT(pOutIndex);

    BlockList* pBlock = NULL;
    if (m_pFreeList == NULL) {
        size_t sz = (m_Capacity + m_ExpandCount) * m_CellSize;
        uint8_t* pNew = reinterpret_cast<uint8_t*>(realloc(m_pBuffer, sz));
        if (pNew == NULL) {
            OUTPUT_ERROR_TRACE("realloc failed: %d Bytes", sz);
            return NULL;
        }
        m_pBuffer = pNew;
        m_pFreeList = InitBuffer(
            m_pBuffer + m_Capacity * m_CellSize, m_ExpandCount, m_Capacity);
        m_Capacity += m_ExpandCount;
    }
    pBlock = m_pFreeList;
    CLEAR_FLAG(pBlock->Header.Flags, FREE_BLOCK_FLAG);
    m_pFreeList = m_pFreeList->pNext;

    *pOutIndex = pBlock->Header.Index;
    return reinterpret_cast<uint8_t*>(pBlock) + sizeof(BlockHeader);
}

void CTable::Release(tIndex idx)
{
    ASSERT(idx < m_Capacity);

    BlockList* pBlock = reinterpret_cast<BlockList*>(m_pBuffer + idx * m_CellSize);
    if (!TEST_FLAG(pBlock->Header.Flags, FREE_BLOCK_FLAG)) {
        pBlock->pNext = m_pFreeList;
        m_pFreeList = pBlock;
        SET_FLAG(pBlock->Header.Flags, FREE_BLOCK_FLAG);
        return;
    }
    ASSERT(false);
}

CTable::BlockList*
CTable::InitBuffer(uint8_t* pBuf, size_t cellCount, size_t baseIndex)
{
    uint8_t* pCur = pBuf;
    BlockList* pBlock = NULL;
    while (cellCount > 0) {
        pBlock = reinterpret_cast<BlockList*>(pCur);
        pBlock->Header.Flags = 0;
        SET_FLAG(pBlock->Header.Flags, FREE_BLOCK_FLAG);
        pBlock->Header.Index = baseIndex;
        pCur += m_CellSize;
        pBlock->pNext = reinterpret_cast<BlockList*>(pCur);
        --cellCount;
        ++baseIndex;
    }
    pBlock->pNext = NULL;
    return reinterpret_cast<BlockList*>(pBuf);
}
