/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpProxyPref.h"
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "HttpDefs.h"
#include "Tracker/Trace.h"

using std::strlen;
using std::strchr;
using std::memcpy;
using std::strerror;
using std::atoi;

CHttpProxyPref::CHttpProxyPref() :
    m_pAuthority(NULL),
    m_WhiteList(sizeof(IPv4AddressIndex), 8)
{
}

CHttpProxyPref::~CHttpProxyPref()
{
}

CHttpProxyPref* CHttpProxyPref::CreateInstance(tConfigNode* pData, CLazyBuffer& buffer)
{
    CHttpProxyPref* pInstance = new CHttpProxyPref();
    if (pInstance) {
        if (!pInstance->Initialize(pData, buffer)) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

bool CHttpProxyPref::IsInWhiteList(tNetworkAddress* pHost)
{
    if (pHost->IsIPv4()) {
        uint32_t addr = pHost->Value.V4;
        return IsInV4WhiteList(addr);
    }
    uint8_t* pIPv6 = pHost->Value.V6;
    return IsInV6WhiteList(pIPv6);
}

bool CHttpProxyPref::IsSocketInWhiteList(sockaddr* pAddress)
{
    tNetworkAddress address;
    if (pAddress->sa_family == AF_INET) {
        address.SetValue4(
            reinterpret_cast<sockaddr_in*>(pAddress)->sin_addr.s_addr);
    } else {
        address.SetValue6(
            reinterpret_cast<sockaddr_in6*>(pAddress)->sin6_addr.s6_addr);
    }
    return IsInWhiteList(&address);
}

bool CHttpProxyPref::AppendWhiteListAddress(
    uint32_t address, uint8_t maskBits, uint32_t addressEnd /* = 0 */)
{
    ASSERT(address > 0);
    ASSERT(maskBits > 0 && maskBits < 32, "maskBits: %u\n", maskBits);
    ASSERT(addressEnd == 0 || addressEnd > address, "addressEnd: %u, address: %u\n", addressEnd, address);

    static const uint32_t maskMap[] = {
        0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
        0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
        0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
        0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
        0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
        0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
        0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
        0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,
    };

    IPv4AddressIndex addrIndexItem;
    addrIndexItem.bRange = addressEnd > 0;
    addrIndexItem.MaskBits = maskBits;
    addrIndexItem.NetworkAddress = (address & maskMap[maskBits - 1]) >> maskBits;
    addrIndexItem.NetworkAddressEnd = (addressEnd & maskMap[maskBits - 1]) >> maskBits;
    return m_WhiteList.PushBack(&addrIndexItem);
}

// whitelist form:
//      192.168.0.0/16
//      172.16.0.0-172.31.0.0/12
bool CHttpProxyPref::AnalyzeWhiteList(const char* pValue)
{
    char buffer[64];
    size_t len = strlen(pValue);
    if (len >= sizeof(buffer)) {
        OUTPUT_WARNING_TRACE("Address is too long. %s\n", pValue);
        return false;
    }
    memcpy(buffer, pValue, len + 1);

    struct in_addr addr;
    uint32_t address = 0;
    uint8_t maskBits = 0;
    uint32_t addressEnd = 0;
    char* pMaskLabel = strchr(buffer, '/');
    if (pMaskLabel == NULL) {
        OUTPUT_WARNING_TRACE("Invalid network address: %s\n", pValue);
        return false;
    }
    *pMaskLabel = '\0';
    maskBits = atoi(pMaskLabel + 1);
    char* pRangeLabel = strchr(buffer, '-');
    if (pRangeLabel) {
        *pRangeLabel = '\0';
        if (!inet_aton(pRangeLabel + 1, &addr)) {
            OUTPUT_WARNING_TRACE("inet_aton: address string invalid: %s\n", pRangeLabel + 1);
            return false;
        }
        addressEnd = ntohl(addr.s_addr);
    }
    if (!inet_aton(buffer, &addr)) {
        OUTPUT_WARNING_TRACE("inet_aton: address string invalid: %s\n", buffer);
        return false;
    }
    address = ntohl(addr.s_addr);
    return AppendWhiteListAddress(address, maskBits, addressEnd);
}

bool CHttpProxyPref::IsInV4WhiteList(uint32_t ipv4)
{
    bool bFound = false;
    uint32_t ipNum = ntohl(ipv4);

    for (size_t i = 0; i < m_WhiteList.Count(); ++i) {
        IPv4AddressIndex* pIndex = reinterpret_cast<IPv4AddressIndex*>(m_WhiteList.At(i));
        ipNum = ((ipNum & pIndex->NetworkMask) >> pIndex->MaskBits);
        bFound = (pIndex->bRange) ?
            ipNum >= pIndex->NetworkAddress && ipNum <= pIndex->NetworkAddressEnd :
            ipNum == pIndex->NetworkAddress;
        if (bFound) {
            break;
        }
    }
    return bFound;
}

// TODO: Impl
bool CHttpProxyPref::IsInV6WhiteList(uint8_t* pIPv6)
{
    return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// TODO: Following functions should be replaced by XML auto descerializer.
//
///////////////////////////////////////////////////////////////////////////////
bool CHttpProxyPref::Initialize(tConfigNode* pData, CLazyBuffer& buffer)
{
    ASSERT(m_pAuthority == NULL);

    bool bRes = false;
    CXMLData::XMLElement* pElem = pData->GetElement();
    CForwardList::Iterator iter = pElem->AttrList.Begin();
    CForwardList::Iterator iterEnd = pElem->AttrList.End();
    while (iter != iterEnd) {
        CXMLData::XMLAttribute* pAttr =
            reinterpret_cast<CXMLData::XMLAttribute*>(pElem->AttrList.DataAt(iter));
        if (strcasecmp(pAttr->pName, "uri") == 0) {
            CUriBuilder builder;
            m_pAuthority = builder.CreateAuthorityByString(pAttr->pValue, &buffer);
            if (m_pAuthority) {
                tNetworkAddress* pHostAddr = m_pAuthority->GetIPAddress();
                if (pHostAddr) {
                    NSNetworkAddress::GetSockAddress(
                        &m_SockAddr, pHostAddr,
                        m_pAuthority->GetPort(DEFAULT_HTTP_PORT_NUM));
                    bRes = true;
                }
            }
        }

        if (!bRes) {
            return false;
        }
        ++iter;
    }

    tConfigRoot::CDFSTraverser traverse(pData);
    tConfigNode* pNode = traverse.GetNext();
    while (pNode) {
        CXMLData::XMLElement* pElem = pNode->GetElement();
        bRes = AnalyzeWhiteList(pElem->pTextValue);
        if (!bRes) {
            break;
        }
        pNode = traverse.GetNext();
    }
    return bRes;
}
