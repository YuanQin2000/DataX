/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __ERROR_NO_H__
#define __ERROR_NO_H__

#include "Thread/Thread.h"

#define ERROR_CODE  CThread::GetErrorNo()
#define SET_ERROR_CODE(code) CThread::SetErrorNo(code)

enum ErrorCode {
    EC_MIN = 999,
    EC_SUCCESS = 1000,
    EC_INPROGRESS,
    EC_INACTIVE,
    EC_PROTOCOL_MALFORMAT,
    EC_PROTOCOL_ERROR,
    EC_CONNECT_FAILED,
    EC_IO_ERROR,
    EC_NO_MEMORY,
    EC_ILLEGAL_PARAMETER,
    EC_UNKNOWN,
    EC_MAX
};

ErrorCode GetStandardErrorCode(int err);
const char* GetErrorPhrase(ErrorCode error);

#endif