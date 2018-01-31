/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CmdHandler.h"
#include "Tracker/Trace.h"

CCmdHandler::CCmdHandler()
{
    ResetStatus();
}

CCmdHandler::~CCmdHandler()
{
}

void CCmdHandler::ResetStatus()
{
    m_bQuit = false;
    m_Error = CEC_OK;
}

const char* CCmdHandler::GetErrorPhrase(CmdErrorCode error)
{
    ASSERT(IsValidError(error));

    static const char* s_Phrases[] = {
        "OK",
        "Parameter Invalid",
        "Parameter Too Long",
        "Command Not Found",
        "File Error",
        "No Memory",
        "IO Error",
        "Message Error",
        "Internal Error",
        "Server Error",
    };

    return s_Phrases[error];
}
