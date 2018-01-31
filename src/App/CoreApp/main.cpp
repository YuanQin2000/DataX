/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "ServerIf/CliService.h"

int main(int argc, char* argv[])
{
    InitializeDebug();

    CCliService::Instance()->Start();
    CCliService::Instance()->WaitStopped();
    return 0;
}
