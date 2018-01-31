/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __BOLT_CHAIN_H__
#define __BOLT_CHAIN_H__

#include "Common/Typedefs.h"
#include <stdarg.h>

struct ArrayDataFrames;
class IBolt;

class CBoltChain
{
public:
    static CBoltChain* CreateInstance(va_list args);

    ~CBoltChain();

    /**
     * @brief Process the data.
     * @param pData The data to be processed.
     * @return The output message if generated, NULL otherwise.
     * @warning The ownership of @pData will be transferred to process.
     */
    ArrayDataFrames* Process(ArrayDataFrames* pData);

private:
    CBoltChain(IBolt** pBoltArray, size_t arrayLen) :
        m_pBoltArray(pBoltArray),
        m_ArrayLen(arrayLen) {}

private:
    IBolt** m_pBoltArray;    // Owned
    size_t m_ArrayLen;

    DISALLOW_DEFAULT_CONSTRUCTOR(CBoltChain);
    DISALLOW_COPY_CONSTRUCTOR(CBoltChain);
    DISALLOW_ASSIGN_OPERATOR(CBoltChain);
};

#endif