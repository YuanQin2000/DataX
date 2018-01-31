/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MQTT_PROCESSOR_H__
#define __MQTT_PROCESSOR_H__

#include "Common/Typedefs.h"
#include "Common/Singleton.h"
#include "IO/Poller.h"

class CMqttProcessor :
    public CPoller::IExtCmdHandler,
    public CSingleton2<CMqttProcessor>
{
public:
    // From CPoller::IExtCmdHandler
    void OnMessage(void* pMsg);

    

private:
    CMqttProcessor() : m_pPoller(NULL) {}
    ~CMqttProcessor()
    {
        delete m_pPoller;
    }

    // From CSingleton2
    bool Initialize()
    {
        m_pPoller = CPoller::CreateInstance(this);
        return m_pPoller != NULL;
    }

private:
    CPoller* m_pPoller; // Owned

    friend class CSingleton2<CMqttProcessor>;
};

#endif