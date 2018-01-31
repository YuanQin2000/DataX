/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpPrefManager.h"
#include "Config/ConfigLoader.h"

CHttpPrefManager::CHttpPrefManager() :
    m_Buffer(),
    m_RequestPref(m_Buffer),
    m_ConnectionConfig(m_Buffer)
{
    tConfigNode* pRequestConfigData = NULL;
    tConfigNode* pConnectionConfigData = NULL;
    CConfigLoader* pLoader = CConfigLoader::CreateInstance("http.xml");

    if (pLoader) {
        pRequestConfigData = pLoader->GetConfigSection("request-preference");
        pConnectionConfigData = pLoader->GetConfigSection("connection");
    }
    m_RequestPref.Initialize(pRequestConfigData);
    m_ConnectionConfig.Initialize(pConnectionConfigData);
    delete pLoader;
}

CHttpPrefManager::~CHttpPrefManager()
{
}
