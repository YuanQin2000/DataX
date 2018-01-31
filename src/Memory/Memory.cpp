/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Memory.h"

class CSystemMemory : public CMemory
{
public:
    CSystemMemory() : CMemory(~0) {}
    void* Malloc(size_t size) { return malloc(size); }
    void Free(void* pMem) { free(pMem); }
};

CMemory* CMemory::GetDefaultMemory()
{
    static CSystemMemory s_SystemMemory;
    return &s_SystemMemory;
}
