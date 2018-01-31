/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ByteData.h"

void CByteData::SetData(
    void* pData, size_t len, tCByteDataReleaseFunc func /* = NULL */)
{
    if (m_pReleaser && m_pData) {
        m_pReleaser(m_pData);
    }
    m_pData = pData;
    m_Length = len;
    m_pReleaser = func;
}

void CByteData::Replace(CByteData* pData)
{
    if (pData) {
        SetData(pData->m_pData, pData->m_Length, pData->m_pReleaser);
        pData->m_pData = NULL;
        pData->m_Length = 0;
        pData->m_pReleaser = NULL;
    } else {
        SetData(NULL, 0, NULL);
    }
}
