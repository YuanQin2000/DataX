/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "IOHelper.h"
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Common/ErrorNo.h"
#include "Tracker/Trace.h"

bool NSIOHelper::SetIOBlockMode(tIOHandle fd, bool bBlocked)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0) {
        OUTPUT_ERROR_TRACE("fcntl: %s\n", strerror(errno));
        return false;
    }

    int newOpts = bBlocked ? (opts & ~O_NONBLOCK) : (opts | O_NONBLOCK);
    if (newOpts != opts && fcntl(fd, F_SETFL, newOpts) < 0) {
        OUTPUT_ERROR_TRACE("fcntl: %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool NSIOHelper::IsIOBlocked(tIOHandle fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0) {
        OUTPUT_ERROR_TRACE("fcntl: %s\n", strerror(errno));
        return false;
    }
    return !(opts & O_NONBLOCK);
}

size_t NSIOHelper::Read(tIOHandle io, void* buf, size_t len)
{
    int res = 0;
    do {
        res = read(io, buf, len);
        if (res >= 0) {
            break;
        }
        if (errno != EINTR) {
            SET_ERROR_CODE(errno);
            res = 0;
            break;
        }
        // EINTR
    } while (true);
    return static_cast<size_t>(res);
}

size_t NSIOHelper::Write(tIOHandle io, void* buf, size_t len)
{
    size_t totalWriteBytes = 0;
    int res = 0;

    while (totalWriteBytes < len) {
        size_t toWriteBytes = len - totalWriteBytes;

        res = write(io, buf, toWriteBytes);
        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            SET_ERROR_CODE(errno);
            break;
        }
        totalWriteBytes += res;
    }
    return totalWriteBytes;
}

long int NSIOHelper::GetFileSize(FILE* fp)
{
    ASSERT(fp != NULL);

    long int res = -1;
    fpos_t pos;
    if (fgetpos(fp, &pos) == 0) {
        fseek(fp, 0L, SEEK_END);
        res = ftell(fp);
        fsetpos(fp, &pos);
    }
    return res;
}
