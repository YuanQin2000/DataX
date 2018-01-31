/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "ConfigLoader.h"
#include <cstring>
#include <linux/limits.h>
#include "EnvManager.h"
#include "Tracker/Trace.h"
#include "XML/XMLParserSAX.h"

using std::memcpy;
using std::strlen;

CConfigLoader::CConfigLoader(const char* pFileName) :
    m_pFileName(pFileName),
    m_pConfig(NULL, CXMLData::DestroyInstance)
{
    ASSERT(m_pFileName);
}

CConfigLoader::~CConfigLoader()
{
}

CConfigLoader* CConfigLoader::CreateInstance(const char* pFileName)
{
    CConfigLoader* pInstance = new CConfigLoader(pFileName);
    if (pInstance) {
        pInstance->m_pConfig = pInstance->Load();
        if (pInstance->m_pConfig.get() == NULL) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

tConfigNode* CConfigLoader::GetConfigSection(const char* pSectionName)
{
    ASSERT(pSectionName);

    if (m_pConfig.get()) {
        tConfigRoot& root = m_pConfig->GetXMLTree();
        CXMLData::XMLElement elem(pSectionName);
        return root.GetRoot()->Find(elem, 1);
    }
    return NULL;
}

shared_ptr<CXMLData> CConfigLoader::Load()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    char filenameBuffer[PATH_MAX];
    const char* pFileFullName = filenameBuffer;
    size_t dirLen = CEnvManager::Instance()->ConfigPathLength();
    size_t filenameLen = strlen(m_pFileName);

    ASSERT(dirLen > 0);
    ASSERT(dirLen + filenameLen + 2 < sizeof(filenameBuffer));

    memcpy(filenameBuffer, CEnvManager::Instance()->ConfigPath(), dirLen);
    if (filenameBuffer[dirLen - 1] != '/') {
        filenameBuffer[dirLen++] = '/';
    }
    memcpy(filenameBuffer + dirLen, m_pFileName, filenameLen + 1);

    CXMLParserSAX xmlParser;
    xmlParser.RegisterAnalyzedElement("configure");
    if (xmlParser.ParseFile(pFileFullName)) {
        return xmlParser.GetXMLData();
    }
    return NULL;
}
