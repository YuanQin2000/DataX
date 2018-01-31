/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "MqttClient.h"
#include "Common/ByteData.h"
#include "DataCom/ConnectionRunner.h"
#include "Thread/ArrayDataFrames.h"
#include "IO/SSLClient.h"
#include "Network/Address.h"
#include "Network/DNSClient.h"
#include <cstring>

using std::strncpy;

CConnectionRunner* CMqttClient::s_pMqttRunner =
    CConnectionRunner::CreateInstance("default-mqtt-client-stack");

CMqttClient::CMqttClient(
    CSSLClient* pSSL,
    sockaddr* pAddr,
    const char* pClientID,
    const char* pThingName) :
    m_pLink(NULL)
{
    ASSERT(pClientID);
    ASSERT(pThingName);

    strncpy(m_ClientID, pClientID, sizeof(m_ClientID) - 1);
    m_ClientID[sizeof(m_ClientID) - 1] = '\0';
    strncpy(m_ThingName, pThingName, sizeof(m_ThingName) - 1);
    m_ThingName[sizeof(m_ThingName) - 1] = '\0';

    if (pAddr) {
        memcpy(&m_ServerAddr, pAddr, sizeof(m_ServerAddr));
    }
}

CMqttClient::~CMqttClient()
{
}

CMqttClient* CMqttClient::CreateInstance(MqttParam* pParam)
{
    ASSERT(pParam);
    ASSERT(pParam->pServerName);

    tNetworkAddress serverAddr(
        CDnsClient::Instance()->QueryIPv4Address(pParam->pServerName));
    if (!serverAddr.IsValid()) {
        OUTPUT_ERROR_TRACE("Can not get server address.\n");
        return NULL;
    }

    CMqttClient* pInstance = new CMqttClient(NULL, pParam->pClientID, pParam->pThingName);
    if (pInstance) {
        NSNetworkAddress::GetSockAddress(&pInstance->m_ServerAddr, &serverAddr, DEFAULT_SSL_PORT);
        CSSLClient* pSSL = NULL;
        CTcpClient* pTcp = new CTcpClient(&pInstance->m_ServerAddr, false); // non-blocking mode
        if (pTcp) {
            pSSL = CSSLClient::CreateInstance(pTcp, true);
        }
        if (pSSL && pSSL->Open()) {
            pInstance->m_pLink = new CConnection();
        } else {
            if (pSSL) {
                delete pSSL;
            } else {
                delete pTcp;
            }
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

bool CMqttClient::Serialize(CByteData* pInData, CByteData* pOutData)
{
}

bool CMqttClient::UnSerialize(CByteData* pInData, CByteData* pOutData)
{
}
