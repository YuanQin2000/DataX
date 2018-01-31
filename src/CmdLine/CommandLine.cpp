/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CommandLine.h"
#include <stdio.h>
#include <strings.h>
#include <cstring>
#include <cctype>
#include <cstdlib>
#ifdef __USE_GNU_READLINE__
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "CmdHandler.h"
#include "LineArguments.h"
#include "DummyCmdHandler.h"
#include "IO/IOContext.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::strlen;
using std::isspace;
using std::malloc;
using std::free;

///////////////////////////////////////////////////////////////////////////////
//
// GNU readline auto complete utility
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __USE_GNU_READLINE__

static char* GenerateRequestHint(const char* pText, int state)
{
    // TODO: Impl
    return NULL;
}

static char* GenerateCommandHint(const char* pText, int state)
{
    // TODO: Impl
    return NULL;
}

static char** TextCompletion(const char* pText, int start, int end)
{
    static bool bStarted = false;
    if (start == 0) {
        bStarted = false;
    } else {
        if (!bStarted) {
            bStarted = (NSCharHelper::TrimLeftSpace(rl_line_buffer) != NULL);
        }
    }
    return bStarted ?
        rl_completion_matches(pText, GenerateCommandHint) :
        rl_completion_matches(pText, GenerateRequestHint);
}

#endif




///////////////////////////////////////////////////////////////////////////////
//
// CCommandLine Implemenation
//
///////////////////////////////////////////////////////////////////////////////
CCommandLine::CCommandLine(
    vector<CCmdHandler*>& cmdHandlers, const char* pPrompt) :
    m_CmdHandlers(cmdHandlers),
    m_pPrompt(pPrompt)
{
    ASSERT(pPrompt);

#ifdef __USE_GNU_READLINE__
    rl_attempted_completion_function = TextCompletion;
#else
    memset(m_InputBuffer, 0. sizeof(m_InputBuffer));
#endif
    memset(m_HistoryBuffer, 0, sizeof(m_HistoryBuffer));
}

CCommandLine::~CCommandLine()
{
}

int CCommandLine::Loop()
{
    int res = 0;
    CLineArguments* pArgs = ReadArguments();
    while (pArgs) {
        vector<CCmdHandler*>::iterator iter = m_CmdHandlers.begin();
        vector<CCmdHandler*>::iterator iterEnd = m_CmdHandlers.end();
        CCmdHandler* pHandler = NULL;
        while (iter != iterEnd) {
            pHandler = *iter;
            if (pHandler->HandleCommand(pArgs)) {
                break;
            }
            ++iter;
            pHandler = NULL;
        }

        if (pHandler == NULL) {
            pHandler = CDummyCmdHandler::Instance();
        }

        bool bContinued = true;
        CCmdHandler::CmdErrorCode error = pHandler->GetErrorCode();
        if (error == CCmdHandler::CEC_OK) {
            ShowMsg("OK");
        } else {
            ShowErrorMsg(CCmdHandler::GetErrorPhrase(error));
            if (CCmdHandler::IsFataError(error)) {
                res = -1;
                bContinued = false;
            }
        }

        if (pHandler->IsQuit()) {
            bContinued = false;
        }

        AddHistory(pArgs);
        delete pArgs;
        if (!bContinued) {
            break;
        }
        pHandler->ResetStatus();
        pArgs = ReadArguments();
    }
    return res;
}

CLineArguments* CCommandLine::ReadArguments()
{
    CLineArguments* pArg = NULL;
    char* pLine = GetInputLine();
    if (pLine) {
        pArg = CLineArguments::CreateInstance(pLine);
        if (pArg == NULL) {
            OUTPUT_ERROR_TRACE("Create InputArguments failed.\n");
#ifdef __USE_GNU_READLINE__
            free(pLine);
#endif
        }
    }
    return pArg;
}

void CCommandLine::AddHistory(CLineArguments* pArgs)
{
#ifdef __USE_GNU_READLINE__
    const char* pHistoryLine =
        pArgs->GetArgumentString(m_HistoryBuffer, sizeof(m_HistoryBuffer));
    if (pHistoryLine) {
        add_history(pHistoryLine);
    }
#endif
}

char* CCommandLine::GetInputLine()
{
    char* pLine = NULL;
    bool bContinued = false;

    do {
#ifdef __USE_GNU_READLINE__
        pLine = readline(m_pPrompt);
#else
        size_t readBytes = fread(m_InputBuffer, 1, sizeof(m_InputBuffer) - 1, STDIN);
        if (readBytes <= 0) {
            return NULL;
        }
        if (m_InputBuffer[readBytes - 1]) == '\n') {
            --readBytes;
        }
        m_InputBuffer[readBytes] = '\0';
        pLine = m_InputBuffer;
#endif

        if (pLine == NULL) {
            break;
        }

        if (NSCharHelper::TrimRightSpace(pLine) == NULL) {
            // Blank line
#ifdef __USE_GNU_READLINE__
            free(pLine);
#endif
            pLine = NULL;
            bContinued = true;
        }
    } while (bContinued);
    return pLine;
}
