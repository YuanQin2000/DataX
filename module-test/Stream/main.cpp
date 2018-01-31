/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SimpleFilter.h"
#include "OutputBolt.h"
#include "CaseChange.h"
#include "Common/Macros.h"
#include "Stream/Topology.h"
#include "StreamUtils/TextFileSpout.h"
#include "Tracker/Trace.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_SPOUT 0
#define UPPER_CASE_BOLT 1
#define LOWER_CASE_BOLT 2
#define REVERSE_CASE_BOLT 3
#define RAW_OUTPUT_BOLT 4

int main(int argc, char* argv[])
{
    if (argc != 2) {
        OUTPUT_WARNING_TRACE("Usage: %s <file name>\n", argv[0]);
        return -1;
    }

    InitializeDebug();

    CTopology* pTopology = new CTopology();
    int fd = open(argv[1], O_RDONLY);
    ASSERT(fd >= 0);
    CTextFileSpout* pFileSpout = new CTextFileSpout("\n", fd);
    CSimpleFilterBolt* pFilterBolt = new CSimpleFilterBolt("Cancer");
    CCaseChangeBolt* pUpperCaseBolt = new CCaseChangeBolt(CCaseChangeBolt::CCT_LOWER2UPPER);
    CCaseChangeBolt* pLowerCaseBolt = new CCaseChangeBolt(CCaseChangeBolt::CCT_UPPER2LOWER);
    CCaseChangeBolt* pReverseCaseBolt = new CCaseChangeBolt(CCaseChangeBolt::CCT_REVERSE);
    COutputBolt* pPrintBolt = new COutputBolt();

    DECLARE_SPOUT(pTopology, "rawFile", FILE_SPOUT, *pFileSpout, pFilterBolt);
    DECLARE_BOLT(pTopology, "rawout", RAW_OUTPUT_BOLT, FILE_SPOUT, pPrintBolt);
    DECLARE_BOLT(pTopology, "upper", UPPER_CASE_BOLT, FILE_SPOUT, pUpperCaseBolt, pPrintBolt);
    DECLARE_BOLT(pTopology, "lower", LOWER_CASE_BOLT, FILE_SPOUT, pLowerCaseBolt, pPrintBolt);
    DECLARE_BOLT(pTopology, "reverse", REVERSE_CASE_BOLT, FILE_SPOUT, pReverseCaseBolt, pPrintBolt);

    pTopology->Start();
    pTopology->WaitStopped();

    delete pTopology;
    delete pFileSpout;
    delete pFilterBolt;
    delete pUpperCaseBolt;
    delete pLowerCaseBolt;
    delete pReverseCaseBolt;
    delete pPrintBolt;
    return 0;
}
