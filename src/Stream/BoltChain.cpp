/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "BoltChain.h"
#include "Bolt.h"
#include "Thread/ArrayDataFrames.h"
#include <cstdlib>

using std::free;

CBoltChain::~CBoltChain()
{
    if (m_pBoltArray) {
        free(m_pBoltArray);
    }
}

CBoltChain* CBoltChain::CreateInstance(va_list args)
{
    // Calculate the bolt count.
    va_list argsCopied;
    va_copy(argsCopied, args);
    size_t count = 0;
    IBolt* pBolt = va_arg(argsCopied, IBolt*);
    while (pBolt) {
        pBolt = va_arg(argsCopied, IBolt*);
        ++count;
    }
    va_end(argsCopied);
    if (count == 0) {
        return NULL;
    }

    CBoltChain* pInstance = NULL;
    IBolt** pArray = reinterpret_cast<IBolt**>(malloc(count * sizeof(IBolt*)));
    if (pArray) {
        pInstance = new CBoltChain(pArray, count);
        if (pInstance) {
            for (size_t i = 0; i < count; ++i) {
                IBolt* pItem = va_arg(args, IBolt*);
                ASSERT(pItem);
                pArray[i] = pItem;
            }
        } else {
            free(pArray);
        }
    }
    return pInstance;
}

ArrayDataFrames* CBoltChain::Process(ArrayDataFrames* pData)
{
    size_t index = 0;
    ArrayDataFrames* pCur = pData;
    ArrayDataFrames* pResult = NULL;
    while (index < m_ArrayLen && pCur) {
        IBolt* pBolt = m_pBoltArray[index];
        pResult = pBolt->Process(pCur);
        pCur = pResult;
        ++index;
    }
    return pResult;
}
