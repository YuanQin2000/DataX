/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <strings.h>
#include <vector>
#include "Common/Typedefs.h"
#include "CmdLine/CommandLine.h"
#include "CmdLine/CmdHandler.h"
#include "CmdLine/CommonCmdHandler.h"
#include "CmdLine/DummyCmdHandler.h"
#include "CtrlCmdHandler.h"
#include "Tracker/Trace.h"

using std::vector;

static bool CreateCommandHandlers(vector<CCmdHandler*>& handlers)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CCmdHandler* pBuiltinCmdHandler = new CCommonCmdHandler();
    CCmdHandler* pServCmdHandler = CCtrlCmdHandler::CreateInstance();
    if (pBuiltinCmdHandler != NULL && pServCmdHandler != NULL) {
        handlers.push_back(pBuiltinCmdHandler);
        handlers.push_back(pServCmdHandler);
        return true;
    }
    delete pServCmdHandler;
    delete pBuiltinCmdHandler;
    return false;
}

static void DestroyCommandHandlers(vector<CCmdHandler*>& handlers)
{
    vector<CCmdHandler*>::iterator iter = handlers.begin();
    vector<CCmdHandler*>::iterator iterEnd = handlers.end();
    while (iter != iterEnd) {
        delete *iter;
        ++iter;
    }
}

int main(int argc, char* argv[])
{
    TRACK_FUNCTION_LIFE_CYCLE;

    vector<CCmdHandler*> cmdHandlers;
    if (!CreateCommandHandlers(cmdHandlers)) {
        OUTPUT_ERROR_TRACE("Create Command Handlers Failed\n");
        return -1;
    }

    int res = -1;
    CCommandLine* pCmdline = new CCommandLine(cmdHandlers, "> ");
    if (pCmdline) {
        res = pCmdline->Loop();
        delete pCmdline;
    }
    DestroyCommandHandlers(cmdHandlers);
    return res;
}
