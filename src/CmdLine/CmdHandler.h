/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_CMD_HANDLER_H__
#define __CLIENT_CMD_HANDLER_H__

class CLineArguments;
class CCmdHandler
{
public:
    enum CmdErrorCode {
        CEC_OK = 0,
        CEC_PARAM_INVALID,
        CEC_PARAM_TOO_LONG,
        CEC_CMD_NOT_FOUND,
        CEC_FILE_ERROR,
        CEC_NO_MEMORY,
        CEC_IO_ERROR,
        CEC_MSG_ERROR,
        CEC_LOCAL_ERROR,
        CEC_SERVER_ERROR,
        CEC_COUNT
    };

    CCmdHandler();
    virtual ~CCmdHandler();

    virtual bool HandleCommand(CLineArguments* pArgs) = 0;

    CmdErrorCode GetErrorCode() const { return m_Error; }
    bool IsQuit() const { return m_bQuit; }
    void ResetStatus();

    static bool IsFataError(CmdErrorCode error)  { return error >= CEC_NO_MEMORY; }
    static bool IsValidError(CmdErrorCode error) { return error >= CEC_OK && error < CEC_COUNT; }
    static const char* GetErrorPhrase(CmdErrorCode error);

protected:
    CmdErrorCode m_Error;
    bool m_bQuit;
};

#endif