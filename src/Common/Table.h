/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_TABLE_H__
#define __COMMON_TABLE_H__

#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Tracker/Trace.h"
#include <cstdlib>

using std::free;

class CTable
{
public:
    typedef uint32_t tIndex;

    CTable(size_t expandCount, size_t cellSize = 0) :
        m_pBuffer(NULL),
        m_pFreeList(NULL),
        m_Capacity(0),
        m_CellSize(cellSize > sizeof(void*) ? cellSize + sizeof(BlockHeader) : sizeof(BlockList)),
        m_ExpandCount(expandCount),
        m_bAggregated(cellSize > 0) {}

    ~CTable()
    {
        if (m_pBuffer) {
            free(m_pBuffer);
        }
    }

    bool IsAggregated() const { return m_bAggregated; }

    void* At(tIndex idx);
    void Set(tIndex idx, void* pData);
    void* Allocate(tIndex* pOutIdx);
    void Release(tIndex idx);

private:
    struct BlockHeader {
        uint8_t Flags;
        uint8_t Reserved[3];
        uint32_t Index;
    };

    struct BlockList {
        BlockHeader Header;
        BlockList* pNext;
    };

    BlockList* InitBuffer(uint8_t* pBuf, size_t cellCount, size_t baseIndex);

private:
    uint8_t* m_pBuffer;
    BlockList* m_pFreeList;
    size_t m_Capacity;
    const size_t m_CellSize;
    const size_t m_ExpandCount;
    const bool m_bAggregated;

    static const uint8_t FREE_BLOCK_FLAG = 0x01;

    DISALLOW_DEFAULT_CONSTRUCTOR(CTable);
    DISALLOW_COPY_CONSTRUCTOR(CTable);
    DISALLOW_ASSIGN_OPERATOR(CTable);
};

#endif