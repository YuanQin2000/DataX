/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COMMON_IO_HELPER_H__
#define __COMMON_IO_HELPER_H__

#include "Common/Typedefs.h"
#include <stdio.h>

namespace NSIOHelper
{
    bool SetIOBlockMode(tIOHandle fd, bool bBlocked);
    bool IsIOBlocked(tIOHandle fd);
    size_t Read(tIOHandle io, void* buf, size_t bytes);
    size_t Write(tIOHandle io, void* buf, size_t bytes);

    long int GetFileSize(FILE* fp);
};

#endif
