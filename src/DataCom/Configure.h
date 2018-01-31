/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATA_COM_CONFIGURE_H__
#define __DATA_COM_CONFIGURE_H__

#include "Common/Typedefs.h"

class CRequest;
class CController;
class CConfigure
{
public:
    CConfigure(size_t inBufSize, size_t outBufSize) :
        m_InBufferSize(inBufSize), m_OutBufferSize(outBufSize) {}
    virtual ~CConfigure() {}

    size_t InBufferSize() const { return m_InBufferSize; }
    size_t OutBufferSize() const { return m_OutBufferSize; }

    virtual bool CreateController(CController** pOutController, CRequest* pRequest) = 0;

private:
    const size_t m_InBufferSize;
    const size_t m_OutBufferSize;
};

#endif