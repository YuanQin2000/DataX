/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_IF_IPC_H__
#define __CLIENT_IF_IPC_H__

extern const char* g_pSocketAddress;

class CIOContext;
CIOContext* CreateClient();
void DestroyClient(CIOContext* pIOContext);

#endif