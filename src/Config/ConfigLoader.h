/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <memory>
#include "Tracker/Trace.h"
#include "ConfigObject.h"
#include "XML/XMLData.h"

using std::shared_ptr;

class CConfigLoader
{
public:
    struct KeyValue {
        KeyValue(const char* pK) : pKey(pK), pValue(NULL) {}
        const char* pKey;
        const char* pValue;
    };

    ~CConfigLoader();

    tConfigRoot* GetConfigRoot() { return m_pConfig ? &m_pConfig->GetXMLTree() : NULL; }
    tConfigNode* GetConfigSection(const char* pSectionName);

    static CConfigLoader* CreateInstance(const char* pFileName);

private:
    CConfigLoader(const char* pFileName);

    shared_ptr<CXMLData> Load();

private:
    const char* m_pFileName;
    shared_ptr<CXMLData> m_pConfig;
};

#endif