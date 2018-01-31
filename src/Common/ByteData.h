/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_BYTE_DATA_H__
#define __COMMON_BYTE_DATA_H__

#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

typedef void (*tCByteDataReleaseFunc)(void*);

class CByteData
{
public:
    CByteData(
        void* pData = NULL,
        size_t len = 0,
        tCByteDataReleaseFunc func = NULL) :
        m_pData(pData),
        m_Length(len),
        m_pReleaser(func)
    {
        ASSERT(pData || (pData == NULL && len == 0 && func == NULL));
    }

    CByteData(const CByteData& rhs) :
        m_pData(rhs.m_pData),
        m_Length(rhs.m_Length),
        m_pReleaser(NULL)
    {
        ASSERT(rhs.m_pReleaser == NULL);
    }

    ~CByteData()
    {
        if (m_pReleaser && m_pData) {
            m_pReleaser(m_pData);
        }
    }

    CByteData& operator=(const CByteData& rhs)
    {
        ASSERT(rhs.m_pReleaser == NULL);
        SetData(rhs.m_pData, rhs.m_Length);
        return *this;
    }

    void* GetData() const    { return m_pData;  }
    size_t GetLength() const { return m_Length; }
    bool IsNull() const { return m_pData == NULL; }
    void Reset() { SetData(NULL, 0, NULL); }

    void SetData(void* pData, size_t len, tCByteDataReleaseFunc func = NULL);
    void Replace(CByteData* pData);

private:
    void* m_pData;
    size_t m_Length;
    tCByteDataReleaseFunc m_pReleaser;
};

#endif