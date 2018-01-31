/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DYNAMIC_BUFFER_H__
#define __DYNAMIC_BUFFER_H__

#include "Common/Typedefs.h"

class CDynamicBuffer
{
public:
    struct DataBlock {
        DataBlock(uint8_t* pBuffer, size_t length) :
            pData(pBuffer), Length(length), pNext(NULL) {}
        uint8_t*   pData;
        size_t     Length;
        DataBlock* pNext;
    };

public:
    static CDynamicBuffer* CreateInstance(size_t len);
    static void DestroyInstance(CDynamicBuffer* pInstance);

    DataBlock* CreateBlock(size_t len);

    DataBlock* GetFirstBlock() { return &m_First; }

private:
    CDynamicBuffer(uint8_t* pBuffer, size_t len);
    ~CDynamicBuffer();

private:
    DataBlock m_First;
    DataBlock* m_pLast;

    DISALLOW_COPY_CONSTRUCTOR(CDynamicBuffer);
    DISALLOW_ASSIGN_OPERATOR(CDynamicBuffer);
    DISALLOW_DEFAULT_CONSTRUCTOR(CDynamicBuffer);
};

#endif