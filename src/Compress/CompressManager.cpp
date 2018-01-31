/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CompressManager.h"
#include "Tracker/Trace.h"
#include "LZWWrapper.h"
#include "DeflatWrapper.h"
#include "GzipWrapper.h"

CCompressManager::CCompressManager()
{
    m_RegisteredProcessor[CT_COMPRESS].bActive = false;
    m_RegisteredProcessor[CT_DEFLAT].bActive = false;
    m_RegisteredProcessor[CT_GZIP].bActive = true;

    m_RegisteredProcessor[CT_COMPRESS].ComCreator = CLzwCompressWrapper::CreateInstance;
    m_RegisteredProcessor[CT_COMPRESS].DecomCreator = CLzwDecompressWrapper::CreateInstance;
    m_RegisteredProcessor[CT_DEFLAT].ComCreator = CDeflatCompressWrapper::CreateInstance;
    m_RegisteredProcessor[CT_DEFLAT].DecomCreator = CDeflatDecompressWrapper::CreateInstance;
    m_RegisteredProcessor[CT_GZIP].ComCreator = CGzipCompressWrapper::CreateInstance;
    m_RegisteredProcessor[CT_GZIP].DecomCreator = CGzipDecompressWrapper::CreateInstance;
}

CCompressManager::~CCompressManager()
{
}

vector<CompressType>& CCompressManager::GetSupportedCompressType()
{
    static bool s_bInited = false;
    static vector<CompressType> s_CompressTypes(CT_COUNT);
    if (!s_bInited) {
        s_bInited = true;
        size_t idx = 0;
        for (size_t i = 0; i < CT_COUNT; ++i) {
            if (m_RegisteredProcessor[i].bActive) {
                s_CompressTypes[idx] = static_cast<CompressType>(i);
                ++idx;
            } else {
                s_CompressTypes.pop_back();
            }
        }
    }
    return s_CompressTypes;
}

vector<CompressType>& CCompressManager::GetSupportedDecompressType()
{
    static bool s_bInited = false;
    static vector<CompressType> s_DecompressTypes(CT_COUNT);
    if (!s_bInited) {
        s_bInited = true;
        size_t idx = 0;
        for (size_t i = 0; i < CT_COUNT; ++i) {
            if (m_RegisteredProcessor[i].bActive) {
                s_DecompressTypes[idx] = static_cast<CompressType>(i);
                ++idx;
            } else {
                s_DecompressTypes.pop_back();
            }
        }
    }
    return s_DecompressTypes;
}
