/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_LAZY_BUFFER_H__
#define __COMMON_LAZY_BUFFER_H__

#include <cstring>
#include "Common/Typedefs.h"
#include "Memory.h"

using std::strlen;

class CLazyBuffer : public CMemory
{
public:
    CLazyBuffer(uint8_t* pBuffer, size_t length);
    CLazyBuffer(size_t length = DEFAULT_SIZE_OF_BUFFER) :
        CMemory(~0),
        m_pHeader(NULL),
        m_MinBlockSize(length),
        m_bExtentable(true) {}
    ~CLazyBuffer();

    // From CMemory
    void* Malloc(size_t length);
    void Free(void* pMem);

    void Reset();

    const char* StoreNString(const char* pString, size_t len);
    const char* StoreString(const char* pString)
    {
        return StoreNString(pString, strlen(pString));
    }

#ifdef __DEBUG__
    void DumpAll();
#endif

private:
    struct BlockList {
        BlockList* pNext;
        uint8_t*   pBuffer;
        uint8_t*   pFree;
        size_t     FreeSize;

        BlockList(uint8_t* pBuf, size_t length) :
            pNext(NULL),
            pBuffer(pBuf),
            pFree(pBuf),
            FreeSize(length) {}
    };

    BlockList* FindFreeBlock(size_t length);

    BlockList* m_pHeader;
    const size_t m_MinBlockSize;
    const bool m_bExtentable;

    static const size_t DEFAULT_SIZE_OF_BUFFER = 1024;

    DISALLOW_COPY_CONSTRUCTOR(CLazyBuffer);
    DISALLOW_ASSIGN_OPERATOR(CLazyBuffer);
};

#endif