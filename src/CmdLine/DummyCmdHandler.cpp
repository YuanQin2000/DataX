/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DummyCmdHandler.h"

bool CDummyCmdHandler::HandleCommand(CLineArguments* pArgs)
{
    m_bQuit = false;
    return true;
}

CDummyCmdHandler::CDummyCmdHandler() : CCmdHandler()
{
    m_Error = CEC_CMD_NOT_FOUND;
}

CDummyCmdHandler::~CDummyCmdHandler()
{
}