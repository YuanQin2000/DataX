/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_MEMORY_POOL_H__
#define __COMMON_MEMORY_POOL_H__

#include <cstring>
#include "Common/Arch.h"
#include "Common/Typedefs.h"
#include "Memory.h"
#include "Tracker/Trace.h"

/**
 * @warning: Not thread safe.
 */
class CMemoryPool : public CMemory
{
public:
    CMemoryPool(size_t cellSize, size_t poolSize = 0);
    ~CMemoryPool();

    // From CMemory
    void* Malloc(size_t size);
    void Free(void* pMem);

    static size_t ReformCellSize(size_t cellSize);

private:
    struct List {
        List() : pNext(0) {}
        List* pNext;
    };

    class CBlock
    {
    public:
        CBlock() : m_pNext(NULL), m_pBuffer(NULL) {}

        ~CBlock()
        {
            delete [] m_pBuffer;
            m_pBuffer = NULL;
        }

        List* Initialize(size_t cellSize, size_t blockSize);

        CBlock* Next() const { return m_pNext; }
        void SetNext(CBlock* pNext) { m_pNext = pNext; }

    private:
        CBlock* m_pNext;
        uint8_t* m_pBuffer;
    };

    List* m_pFree;
    CBlock* m_pBlocks;
    size_t m_PoolSize;
    size_t m_MaxPoolSize;

    static const size_t DEFAULT_BLOCK_SIZE = 4 * 1024; /* 4KB */

#ifdef __DEBUG__
    void DumpFreeCells();
#endif
};

#endif
