/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMPRESS_MANAGER_H__
#define __COMPRESS_MANAGER_H__

#include <vector>
#include "Tracker/Trace.h"
#include "Common/Typedefs.h"
#include "Compressor.h"
#include "Common/Singleton.h"

using std::vector;

class CCompressManager : public CSingleton<CCompressManager>
{
public:
    ~CCompressManager();

    vector<CompressType>& GetSupportedCompressType();
    vector<CompressType>& GetSupportedDecompressType();

    CCompressor* CreateCompressor(CompressType type)
    {
        ASSERT(type >= 0 && type < CT_COUNT);
        return m_RegisteredProcessor[type].ComCreator();
    }
    CDecompressor* CreateDecompressor(CompressType type)
    {
        ASSERT(type >= 0 && type < CT_COUNT);
        return m_RegisteredProcessor[type].DecomCreator();
    }

protected:
    CCompressManager();

private:
    typedef CCompressor* (*tCompressorCreator)();
    typedef CDecompressor* (*tDecompressorCreator)();
    struct ProcessorRegData {
        tCompressorCreator ComCreator;
        tDecompressorCreator DecomCreator;
        bool bActive;
    };

    ProcessorRegData m_RegisteredProcessor[CT_COUNT];

    friend class CSingleton<CCompressManager>;

    DISALLOW_COPY_CONSTRUCTOR(CCompressManager);
    DISALLOW_ASSIGN_OPERATOR(CCompressManager);
};

#endif