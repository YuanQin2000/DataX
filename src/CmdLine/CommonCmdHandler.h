/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CMDLINE_COMMON_CMD_HANDLER_H__
#define __CMDLINE_COMMON_CMD_HANDLER_H__

#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "CmdHandler.h"

class CLineArguments;
class CStringIndex;
class CCommonCmdHandler : public CCmdHandler
{
public:
    CCommonCmdHandler();
    ~CCommonCmdHandler();

    // From CmdHandler
    bool HandleCommand(CLineArguments* pArgs);

private:
    typedef void (*tCmdHandler)(CCommonCmdHandler* pThis, CLineArguments* pArgs);

    enum CommandID {
        CID_QUIT,
        CID_COUNT
    };

    static tCmdHandler GetBuiltinCmdHandler(const char* pName);
    static void HandleQuitCommand(CCommonCmdHandler* pThis, CLineArguments* pArgs);
    static CStringIndex* CreateCommandNameIndex();

    static const char* s_CommandNames[CID_COUNT];
    static const tCmdHandler s_CommandHandlers[CID_COUNT];
    static const CStringIndex* s_NameIndex;
};

#endif