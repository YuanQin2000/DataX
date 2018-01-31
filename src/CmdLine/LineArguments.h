/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CMDLINE_LINE_ARGUMENTS_H__
#define __CMDLINE_LINE_ARGUMENTS_H__

#include "Common/Typedefs.h"
#include "Common/Vector.h"

class CLineArguments
{
public:
    enum ArgumentType {
        AT_STRING = 0,
        AT_FILE,
        AT_COUNT
    };

    struct ArgumentData {
        ArgumentType Type;
        char* pString;

        ArgumentData(ArgumentType type, char* pStr) : Type(type), pString(pStr) {} 
    };

    ~CLineArguments();

    const char* GetOutputFileName() { return m_pFileName; }
    size_t ArgumentsCount() const { return m_ArgData.Count(); }
    void PopFront() { m_ArgData.PopFront(); }
    ArgumentData* GetArgumentAt(size_t i)
    {
        return reinterpret_cast<ArgumentData*>(m_ArgData.At(i));
    }

    const char* GetArgumentString(char* pBuffer, size_t len);
    void AnalyzeOutputFile();

    static CLineArguments* CreateInstance(char* pTextLine, bool bTransferOwnership = false);

private:
    CLineArguments(char* pRawData);

private:
    char* m_pRawData;     // Owned
    const char* m_pFileName;
    CVector m_ArgData;
    CVector m_ArgDataBackup;

    DISALLOW_COPY_CONSTRUCTOR(CLineArguments);
    DISALLOW_ASSIGN_OPERATOR(CLineArguments);
    DISALLOW_DEFAULT_CONSTRUCTOR(CLineArguments);
};

#endif