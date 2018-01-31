/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Macros.h"
#include "Common/CharHelper.h"
#include "Tracker/Debug.h"
#include "Defines.h"
#include <cstring>
#include <errno.h>
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

using std::strlen;
using std::strstr;
using std::strerror;

TEST_GROUP(CharHelper)
{
    const char* m_pFileName = "../data/test-file.txt";

void setup()
{
}

void teardown()
{
}

};


TEST(CharHelper, TestFindSubString)
{
    static const char* s_SubStrings[] = {
        "c",
        "ch",
        "cha",
        "char",
        "strstr"
    };

    FILE* pFileStream = fopen(m_pFileName, "r");
    ASSERT(pFileStream != NULL, "fopen %s: %s\n", m_pFileName, strerror(errno));

    char lineBuffer[256];
    while (fgets(lineBuffer, sizeof(lineBuffer), pFileStream) != NULL) {
        const char* pLineEnd = lineBuffer + strlen(lineBuffer);
        for (size_t i = 0; i < COUNT_OF_ARRAY(s_SubStrings); ++i) {
            size_t subLen = strlen(s_SubStrings[i]);
            const char* pCur = lineBuffer;
            const char* pFound = NULL;
            const char* pFoundBenchMark = NULL;
            do {
                pFound = NSCharHelper::FindSubStr(s_SubStrings[i], subLen, pCur, pLineEnd);
                pFoundBenchMark = strstr(pCur, s_SubStrings[i]);
                LONGS_EQUAL(pFoundBenchMark, pFound);
                if (pFoundBenchMark == NULL) {
                    break;
                }
                pCur = pFoundBenchMark + subLen;
            } while (pCur < pLineEnd);
        }
    }
    fclose(pFileStream);
}
