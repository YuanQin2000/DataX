/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_COMMAND_LINE_H__
#define __CLIENT_COMMAND_LINE_H__

#define __USE_GNU_READLINE__    // TODO: To defined Makefile

#include <cstdio>
#include <vector>
#include "Common/Typedefs.h"

using std::printf;
using std::vector;

class CLineArguments;
class CCmdHandler;
class CCommandLine
{
public:
    CCommandLine(vector<CCmdHandler*>& cmdHandlers, const char* pPrompt);
    ~CCommandLine();

    int Loop();

private:
    CLineArguments* ReadArguments();
    void AddHistory(CLineArguments* pArgs);
    char* GetInputLine();

    // Utilities
    inline void ShowErrorMsg(const char* pErrorMsg)
    {
        printf("Error: %s\n", pErrorMsg);
    }

    inline void ShowMsg(const char* pMsg)
    {
        printf("%s\n", pMsg);
    }

private:
    vector<CCmdHandler*>& m_CmdHandlers;
    const char* m_pPrompt;          // Not owned

#ifndef __USE_GNU_READLINE__
    char m_InputBuffer[8192];
#endif
    char m_HistoryBuffer[1024];

    DISALLOW_DEFAULT_CONSTRUCTOR(CCommandLine);
    DISALLOW_COPY_CONSTRUCTOR(CCommandLine);
    DISALLOW_ASSIGN_OPERATOR(CCommandLine);
};

#endif