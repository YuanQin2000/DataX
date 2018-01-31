/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DynamicBuffer.h"
#include <cstdlib>
#include <cstring>
#include <new>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

using std::memset;
using std::malloc;
using std::free;

CDynamicBuffer* CDynamicBuffer::CreateInstance(size_t len)
{
    ASSERT(len > 0);

    CDynamicBuffer* pInstance = NULL;
    void* pMem = malloc(sizeof(CDynamicBuffer) + len);
    if (pMem) {
        pInstance = new (pMem) CDynamicBuffer(reinterpret_cast<uint8_t*>(
                                        pMem) + sizeof(CDynamicBuffer), len);
    }
    return pInstance;
}

void CDynamicBuffer::DestroyInstance(CDynamicBuffer* pInstance)
{
    ASSERT(pInstance);

    DataBlock* pBlock = pInstance->m_First.pNext;
    while (pBlock) {
        DataBlock* pNext = pBlock->pNext;
        free(pBlock);
        pBlock = pNext;
    }
    free(pInstance);
}

CDynamicBuffer::DataBlock* CDynamicBuffer::CreateBlock(size_t len)
{
    ASSERT(len > 0);

    DataBlock* pBlock = NULL;
    void* pMem = malloc(sizeof(DataBlock) + len);
    if (pMem) {
        pBlock = new (pMem) DataBlock(reinterpret_cast<uint8_t*>(pMem) + sizeof(DataBlock), len);
        m_pLast->pNext = pBlock;
        m_pLast = pBlock;
    }
    return pBlock;
}

CDynamicBuffer::CDynamicBuffer(uint8_t* pBuffer, size_t len) :
    m_First(pBuffer, len)
{
    m_pLast = &m_First;
}

CDynamicBuffer::~CDynamicBuffer()
{
}
