/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "LazyBuffer.h"
#include <cstdlib>
#include <new>
#include "Tracker/Trace.h"

#ifdef __DEBUG__
#include <cstdio>
#endif

using std::malloc;

CLazyBuffer::CLazyBuffer(uint8_t* pBuffer, size_t length) :
    CMemory(length),
    m_pHeader(NULL),
    m_MinBlockSize(length),
    m_bExtentable(false)
{
    ASSERT(pBuffer);
    ASSERT(length > sizeof(BlockList));

    m_pHeader = new (pBuffer) BlockList(
        (pBuffer) + sizeof(BlockList), length - sizeof(BlockList));
}

CLazyBuffer::~CLazyBuffer()
{
    if (m_bExtentable) {
        BlockList* pList = m_pHeader;
        while (pList) {
            BlockList* pTmp = pList->pNext;
            free(pList);
            pList = pTmp;
        }
    }
}

void* CLazyBuffer::Malloc(size_t length)
{
    uint8_t* pMem = NULL;
    BlockList* pList = FindFreeBlock(length);
    if (pList) {
        pMem = pList->pFree;
        pList->pFree += length;
        pList->FreeSize -= length;
    }
    return pMem;
}

void CLazyBuffer::Free(void* pMem)
{
    // Nothing to do.
}

void CLazyBuffer::Reset()
{
    // TODO: Impl;
}

const char* CLazyBuffer::StoreNString(const char* pString, size_t len)
{
    char* pMem = reinterpret_cast<char*>(Malloc(len + 1));
    if (pMem) {
        memcpy(pMem, pString, len);
        pMem[len] = '\0';
    }
    return pMem;
}

CLazyBuffer::BlockList* CLazyBuffer::FindFreeBlock(size_t length)
{
    BlockList* pList = m_pHeader;
    BlockList* pPre = pList;

    while (pList) {
        if (pList->FreeSize >= length) {
            break;
        }
        pPre = pList;
        pList = pList->pNext;
    }
    if (!pList) {
        if (!m_bExtentable) {
            OUTPUT_WARNING_TRACE("Non-extented buffer full\n");
            return NULL; 
        }

        size_t actualSize = (length > m_MinBlockSize) ? length : m_MinBlockSize;
        uint8_t* pMem =
            reinterpret_cast<uint8_t*>(malloc(sizeof(BlockList) + actualSize));
        if (!pMem) {
            OUTPUT_ERROR_TRACE("Can't not allocate memory for store\n");
            return NULL;
        }
        pList = new (pMem) BlockList(pMem + sizeof(BlockList), actualSize);
        if (pPre) {
            pPre->pNext = pList;
        } else {
            m_pHeader = pList;
        }
    }
    return pList;
}

#ifdef __DEBUG__
void CLazyBuffer::DumpAll()
{
    BlockList* pList = m_pHeader;
    while (pList) {
        ASSERT(pList->pBuffer);
        size_t dataSize = pList->pFree - pList->pBuffer;
        for (size_t i = 0; i < dataSize; i++) {
            printf("%02x ", pList->pBuffer[i]);
        }
        printf("\n");
        pList = pList->pNext;
    }
}
#endif