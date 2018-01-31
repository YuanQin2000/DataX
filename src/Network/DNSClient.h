/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __NETWORK_DNS_H__
#define __NETWORK_DNS_H__

#include "Common/Singleton.h"

class CDnsClient : public CSingleton<CDnsClient>
{
public:
    uint32_t QueryIPv4Address(const char* pHostName);

protected:
    CDnsClient();
    ~CDnsClient();

private:
    friend class CSingleton<CDnsClient>;
};

#endif