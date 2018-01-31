/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "Common/Typedefs.h"
#include "DataCom/Controller.h"

class CConnectionRunner;

struct MqttParam {
    const char* pServerName;
    const char* pClientID;
    const char* pThingName;
};

struct sockaddr;
class CMqttClient : public CController
{
public:
    ~CMqttClient();

    static CMqttClient* CreateInstance(MqttParam* pParam);

private:
    CMqttClient(
        sockaddr* pAddr,
        const char* pClientID,
        const char* pThingName);

private:
    char m_ClientID[80];
    char* m_ThingName[20];
    sockaddr m_ServerAddr;

    static const unsigned short DEFAULT_SSL_PORT = 8883;

    static CConnectionRunner* s_pMqttRunner;

    DISALLOW_DEFAULT_CONSTRUCTOR(CMqttClient);
    DISALLOW_COPY_CONSTRUCTOR(CMqttClient);
    DISALLOW_ASSIGN_OPERATOR(CMqttClient);
};

#endif