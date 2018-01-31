/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DNSClient.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include "Tracker/Trace.h"

using std::strerror;

CDnsClient::CDnsClient()
{
}

CDnsClient::~CDnsClient()
{
}

uint32_t CDnsClient::QueryIPv4Address(const char* pHostName)
{
    struct hostent hostInfo;
    struct hostent* pResult;
    char dnsBuffer[1024];
    int errorNo;
    int res = gethostbyname_r(pHostName,
                              &hostInfo,
                              dnsBuffer,
                              sizeof(dnsBuffer),
                              &pResult,
                              &errorNo);
    if (res != 0) {
        OUTPUT_ERROR_TRACE("gethostbyname_r: return error: %s\n", strerror(res));
        return 0;
    }
    if (!pResult) {
        OUTPUT_ERROR_TRACE("gethostbyname_r: %s\n", strerror(errorNo));
        return 0;
    }
    if (hostInfo.h_addrtype != AF_INET) {
        OUTPUT_ERROR_TRACE("Not support non AF_INET type address\n");
        return 0;
    }

    uint32_t value;
    memcpy(&value, hostInfo.h_addr_list[0], sizeof(struct in_addr));
    return value;
}
