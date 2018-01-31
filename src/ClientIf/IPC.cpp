/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "IPC.h"
#include "IO/UDSStream.h"

const char* g_pSocketAddress = "/tmp/bytec-cli";

CIOContext* CreateClient()
{
    CUDSStreamClient* pInstance = new CUDSStreamClient(g_pSocketAddress, true);
    if (pInstance) {
        if (!pInstance->Open()) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

void DestroyClient(CIOContext* pIOContext)
{
    delete pIOContext;
}
