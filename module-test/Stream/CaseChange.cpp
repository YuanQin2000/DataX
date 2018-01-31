/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CaseChange.h"
#include "Common/Typedefs.h"
#include "Thread/ArrayDataFrames.h"
#include "Tracker/Trace.h"
#include <cctype>

using std::isalpha;
using std::tolower;
using std::toupper;
using std::islower;
using std::isupper;

ArrayDataFrames* CCaseChangeBolt::Process(ArrayDataFrames* pIndata)
{
    if (pIndata->Count == 0) {
        ArrayDataFrames::DeleteInstance(pIndata);
        return NULL;
    }

    for (size_t i = 0; i < pIndata->Count; ++i) {
        DataFrame frame;
        pIndata->GetFrame(i, &frame);
        char* pString = reinterpret_cast<char*>(frame.GetData());
        ASSERT(pString);
        for (size_t j = 0; j < frame.Length; ++j) {
            char ch = pString[j];
            if (isalpha(ch)) {
                switch (m_Type) {
                case CCT_LOWER2UPPER:
                    if (islower(ch)) {
                        pString[j] = toupper(ch);
                    }
                    break;
                case CCT_UPPER2LOWER:
                    if (isupper(ch)) {
                        pString[j] = tolower(ch);
                    }
                    break;
                case CCT_REVERSE:
                    if (isupper(ch)) {
                        pString[j] = tolower(ch);
                    } else {
                        pString[j] = toupper(ch);
                    }
                    break;
                default:
                    ASSERT(false);
                    break;
                }
            }
        }
    }
    return pIndata;
}
