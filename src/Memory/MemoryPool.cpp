/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "MemoryPool.h"

///////////////////////////////////////////////////////////////////////////////
//
// CBlock Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CMemoryPool::List*
CMemoryPool::CBlock::Initialize(size_t cellSize, size_t blockSize)
{
    ASSERT(m_pBuffer == NULL);

    m_pBuffer = new uint8_t[blockSize];
    if (!m_pBuffer) {
        OUTPUT_ERROR_TRACE("Can not allocate memory %lu Bytes", blockSize);
        return NULL;
    }

    int count = blockSize / cellSize;
    List* ptr = reinterpret_cast<List*>(m_pBuffer);
    for (int i = 0; i < count; i++) {
        ptr->pNext = reinterpret_cast<List*>(reinterpret_cast<char*>(ptr) + cellSize);
        ptr = ptr->pNext;
    }
    if (count > 0) {
        // To set the next of last node to be NULL
        ptr = reinterpret_cast<List*>(reinterpret_cast<char*>(ptr) - cellSize);
        ptr->pNext = NULL;
    }
    return reinterpret_cast<List*>(m_pBuffer);
}


///////////////////////////////////////////////////////////////////////////////
//
// CMemoryPool Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CMemoryPool::CMemoryPool(size_t cellSize, size_t poolSize /* = 0 */) :
    CMemory(ReformCellSize(cellSize)),
    m_pFree(NULL),
    m_pBlocks(NULL),
    m_PoolSize(0),
    m_MaxPoolSize(poolSize)
{
    ASSERT(cellSize > 0);
    ASSERT(cellSize <= DEFAULT_BLOCK_SIZE);

    if (poolSize > 0) {
        size_t num = m_MaxPoolSize % (DEFAULT_BLOCK_SIZE / MaxChunkSize());
        if (num != 0) {
            m_MaxPoolSize += (DEFAULT_BLOCK_SIZE / MaxChunkSize() - num);
        }
    } else if (poolSize == 0) {
        m_MaxPoolSize = static_cast<size_t>(-1);
    }
}

CMemoryPool::~CMemoryPool()
{
    CBlock* pBlock = m_pBlocks;
    while (pBlock) {
        CBlock* pTemp = pBlock;
        pBlock = pBlock->Next();
        delete pTemp;
    }
}

void* CMemoryPool::Malloc(size_t size)
{
    ASSERT(size <= MaxChunkSize());

    void* pCell = NULL;
    if (m_PoolSize < m_MaxPoolSize) {
        if (m_pFree == NULL) {
            CBlock* pBlock = new CBlock();
            pBlock->SetNext(m_pBlocks);
            m_pBlocks = pBlock;
            m_pFree = pBlock->Initialize(MaxChunkSize(), DEFAULT_BLOCK_SIZE);
            if (m_pFree == NULL) {
                return NULL;
            }
        }

        pCell = m_pFree;
        m_pFree = m_pFree->pNext;
        ++m_PoolSize;
    }
    return pCell;
}

void CMemoryPool::Free(void* pMem)
{
    ASSERT(pMem);

    List* pCellSpace = reinterpret_cast<List*>(pMem);
    pCellSpace->pNext = m_pFree;
    m_pFree = pCellSpace;
    memset(pMem, 0, MaxChunkSize());
    --m_PoolSize;
}

size_t CMemoryPool::ReformCellSize(size_t cellSize)
{
    static const size_t s_AlignBytes = __OS_WORD_SIZE__ / 8;

    if (cellSize < sizeof(List)) {
        // The Cell size must be equal or greater than the node of List,
        // in order to put the List information.
        cellSize = sizeof(List);
    }
    if ((cellSize & (s_AlignBytes - 1)) != 0) {
        cellSize &= ~(s_AlignBytes - 1);
        cellSize += s_AlignBytes;
    }
    return cellSize;
}


#ifdef __DEBUG__
// Dump the free cell information of the blocks, for debugging
void CMemoryPool::DumpFreeCells()
{
    OUTPUT_DEBUG_TRACE("Object size: %lu\n", MaxChunkSize());
    for (List* p = m_pFree; p; p = p->pNext) {
        OUTPUT_DEBUG_TRACE("%p->", p);
    }
    OUTPUT_DEBUG_TRACE("NULL\n");
}
#endif
