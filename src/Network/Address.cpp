/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Address.h"

void NWAddr::SetValue4(uint32_t value)
{
    if (value > 0) {
        SET_FLAG(BitSet, VALID_FLAG);
        SET_FLAG(BitSet, IPV4_FLAG);
        Value.V4 = value;
    }
}

void NWAddr::SetValue6(uint8_t* ipv6)
{
    ASSERT(ipv6);
    SET_FLAG(BitSet, VALID_FLAG);
    CLEAR_FLAG(BitSet, IPV4_FLAG);
    memcpy(Value.V6, ipv6, sizeof(Value.V6));
}

namespace NSNetworkAddress
{

tNetworkAddress GetIPAddress(const char* pIPString, bool bIPv4 /* = true */)
{
    if (bIPv4) {
        struct in_addr inet4Addr;
        int res = inet_pton(AF_INET, pIPString, &inet4Addr);
        ASSERT(res == 1);
        return tNetworkAddress(inet4Addr.s_addr);
    }

    struct in6_addr inet6Addr;
    int res = inet_pton(AF_INET6, pIPString, &inet6Addr);
    ASSERT(res == 1);
    return tNetworkAddress(inet6Addr.s6_addr, 0);
}

sockaddr* GetSockAddress(
    sockaddr* pSockAddr,
    tNetworkAddress* pAddr,
    unsigned short port)
{
    ASSERT(pSockAddr);
    ASSERT(pAddr->IsValid());

    if (pAddr->IsIPv4()) {
        sockaddr_in* pIn4Addr = reinterpret_cast<sockaddr_in*>(pSockAddr);
        pIn4Addr->sin_family = AF_INET;
        pIn4Addr->sin_addr.s_addr = pAddr->Value.V4;
        pIn4Addr->sin_port = htons(port);
    } else {
        sockaddr_in6* pIn6Addr = reinterpret_cast<sockaddr_in6*>(pSockAddr);;
        pIn6Addr->sin6_family = AF_INET6;
        memcpy(&pIn6Addr->sin6_addr, pAddr->Value.V6, sizeof(pAddr->Value.V6));
        pIn6Addr->sin6_port = htons(port);
    }
    return pSockAddr;
}

in_port_t GetInetSocketPort(const sockaddr* pSockAddr)
{
    ASSERT(pSockAddr);

    if (pSockAddr->sa_family == AF_INET) {
        return reinterpret_cast<const sockaddr_in*>(pSockAddr)->sin_port;
    }
    if (pSockAddr->sa_family == AF_INET6) {
        return reinterpret_cast<const sockaddr_in6*>(pSockAddr)->sin6_port;
    }
    ASSERT(false);
    return 0;
}


};