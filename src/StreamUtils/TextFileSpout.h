/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAMUTILS_TEXT_FILE_SPOUT_H__
#define __STREAMUTILS_TEXT_FILE_SPOUT_H__

#include "Stream/Spout.h"
#include <unistd.h>
#include "Common/Typedefs.h"
#include "Common/OctetBuffer.h"
#include "Tracker/Trace.h"

struct ArrayDataFrames;
class CTextFileSpout : public ISpout
{
public:
    CTextFileSpout(const char* pDelimtor, tIOHandle fd = STDIN_FILENO);
    ~CTextFileSpout();

    // From ISpout
    bool Read(ArrayDataFrames** pOutData);

private:
    tIOHandle m_hIO; // Owned
    COctetBuffer m_Buffer;
    const char* m_pDelimitor;   // Not owned
    size_t m_DelimitorLen;
    size_t m_ScannedLen;
    uint8_t m_BufMem[1024];

    DISALLOW_DEFAULT_CONSTRUCTOR(CTextFileSpout);
    DISALLOW_COPY_CONSTRUCTOR(CTextFileSpout);
    DISALLOW_ASSIGN_OPERATOR(CTextFileSpout);
};

#endif