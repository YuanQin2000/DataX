/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_POINTER_H__
#define __COMMON_POINTER_H__

#include <cstring>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

using std::memcmp;

class CPointer
{
public:
    CPointer(void* pData, size_t size) : m_pData(pData), m_Size(size)
    {
        ASSERT(pData);
        ASSERT(size > 0);
    }

    CPointer(const void* pData, size_t size) :
        m_pData(const_cast<void*>(pData)), m_Size(size)
    {
        ASSERT(pData);
        ASSERT(size > 0);
    }

    CPointer(const CPointer& rhs) : m_pData(rhs.m_pData), m_Size(rhs.m_Size) {}

    ~CPointer() {}

    bool operator==(const CPointer& rhs) const
    {
        return (m_pData == rhs.m_pData ||
            (m_Size == rhs.m_Size && memcmp(m_pData, rhs.m_pData, m_Size) == 0));
    }

    bool operator<(const CPointer& rhs) const
    {
        return m_pData != rhs.m_pData &&
            memcmp(m_pData, rhs.m_pData, m_Size <= rhs.m_Size ? m_Size : rhs.m_Size) < 0;
    }

    CPointer& operator=(const CPointer& rhs)
    {
        m_pData = rhs.m_pData;
        m_Size = rhs.m_Size;
        return *this;
    }

private:
    void* m_pData;
    size_t m_Size;

    DISALLOW_DEFAULT_CONSTRUCTOR(CPointer);
};

#endif