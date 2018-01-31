/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __NETWORK_ADDRESS_H__
#define __NETWORK_ADDRESS_H__

#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "Common/Macros.h"

using std::memcpy;

typedef struct NWAddr {
    uint8_t BitSet;
    union {
        uint32_t V4;
        uint8_t V6[16];     // 128 bits.
    } Value;

public:
    NWAddr(uint32_t ipv4 = 0) : BitSet(0)
    {
        SetValue4(ipv4);
    }

    NWAddr(uint8_t* ipv6, uint8_t placeHolder) : BitSet(0)
    {
        SetValue6(ipv6);
    }

    void SetValue4(uint32_t value);
    void SetValue6(uint8_t* ipv6);

    bool IsValid() const { return TEST_FLAG(BitSet, VALID_FLAG); }
    bool IsIPv4() const  { return TEST_FLAG(BitSet, IPV4_FLAG); }

private:
    static const uint8_t VALID_FLAG = 0x01;
    static const uint8_t IPV4_FLAG = 0x02;
} tNetworkAddress;

namespace NSNetworkAddress
{

tNetworkAddress GetIPAddress(const char* pIPString, bool bIPv4 = true);
sockaddr* GetSockAddress(
    sockaddr* pSockAddr,
    tNetworkAddress* pAddr,
    unsigned short port);
in_port_t GetInetSocketPort(const sockaddr* pSockAddr);

};

#endif