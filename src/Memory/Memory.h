/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_MEMORY_H__
#define __COMMON_MEMORY_H__

#include "Common/Typedefs.h"
#include <cstdlib>

using std::malloc;
using std::free;

class CMemory
{
public:
    CMemory(size_t maxChunkSize) : m_MaxChunkSize(maxChunkSize) {}
    virtual ~CMemory() {}

    virtual void* Malloc(size_t size) = 0;
    virtual void Free(void* pMem) = 0;

    size_t MaxChunkSize() const { return m_MaxChunkSize; }

    static CMemory* GetDefaultMemory();

private:
    const size_t m_MaxChunkSize;
};

#endif