/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CMDLINE_DUMMY_COMMAND_HANDLER_H__
#define __CMDLINE_DUMMY_COMMAND_HANDLER_H__

#include "CmdHandler.h"
#include "Common/Singleton.h"

class CDummyCmdHandler :
    public CCmdHandler,
    public CSingleton<CDummyCmdHandler>
{
public:
    // From CCmdHandler
    bool HandleCommand(CLineArguments* pArgs);

protected:
    CDummyCmdHandler();
    ~CDummyCmdHandler();

private:
    friend class CSingleton<CDummyCmdHandler>;
};

#endif