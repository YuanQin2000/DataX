/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_PROXY_PREF_H__
#define __HTTP_PROXY_PREF_H__

#include <sys/socket.h>
#include "Common/Vector.h"
#include "URI/URI.h"
#include "Network/Address.h"
#include "Config/ConfigObject.h"

class CAuthority;
class CLazyBuffer;
class CHttpProxyPref
{
public:
    ~CHttpProxyPref();

    sockaddr* GetSocketAddress() { return &m_SockAddr; }
    tNetworkAddress* GetHostAddress() { return &m_HostAddr; }

    const char* GetUserName()
    {
        // TODO: impl
        return NULL;
    }
    const char* GetPassword()
    {
        // TODO: impl
        return NULL;
    }

    bool IsInWhiteList(tNetworkAddress* pHost);
    bool IsSocketInWhiteList(sockaddr* pSockAddr);

    static CHttpProxyPref* CreateInstance(tConfigNode* pData, CLazyBuffer& buffer);

private:
    struct IPv4AddressIndex {
        uint32_t NetworkAddress;
        uint32_t NetworkMask;
        uint8_t MaskBits;
        bool bRange;
        uint32_t NetworkAddressEnd; // If range.
    };

    CHttpProxyPref();
    bool Initialize(tConfigNode* pData, CLazyBuffer& buffer);
    bool AnalyzeWhiteList(const char* pValue);

    bool AppendWhiteListAddress(
        uint32_t address, uint8_t maskBits, uint32_t addressEnd = 0);

    bool IsInV4WhiteList(uint32_t ipv4);
    bool IsInV6WhiteList(uint8_t* pIPv6);

private:
    CAuthority* m_pAuthority;
    CVector m_WhiteList;
    sockaddr m_SockAddr;
    tNetworkAddress m_HostAddr;
};

#endif