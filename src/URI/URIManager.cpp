/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "URIManager.h"
#include "Common/CharHelper.h"

using std::pair;

CUriManager::CUriManager() :
    m_CS(),
    m_RegisteredScheme(NSCharHelper::StringCompare)
{
    memset(m_Attributions, 0, sizeof(m_Attributions));
    memset(m_SchemeNames, 0, sizeof(m_SchemeNames));
}

CUriManager::~CUriManager()
{
}

bool CUriManager::GetSchemeID(const char* pString, SchemeID& ID)
{
    ASSERT(pString);

    bool bRes = false;
    map<const char*, SchemeID>::iterator it = m_RegisteredScheme.find(pString);
    if (it != m_RegisteredScheme.end()) {
        ID = it->second;
        bRes = true;
    }
    return bRes;
}

bool CUriManager::GetAttribution(const char* pString, tURIAttributions& attr)
{
    ASSERT(pString);

    bool bRes = false;
    map<const char*, SchemeID>::iterator it = m_RegisteredScheme.find(pString);
    if (it != m_RegisteredScheme.end()) {
        attr = m_Attributions[it->second];
        bRes = true;
    }
    return bRes;
}

bool CUriManager::Register(SchemeID ID, const char* pName, tURIAttributions attr)
{
    ASSERT(ID >= 0 && ID < SCHEME_COUNT);
    ASSERT(pName);

    CSectionLock lock(m_CS);
    pair<map<const char*, SchemeID>::iterator, bool> res
        = m_RegisteredScheme.insert(pair<const char*, SchemeID>(pName, ID));
    if (res.second) {
        m_Attributions[ID] = attr;
        m_SchemeNames[ID] = pName;
    }
    return res.second;
}


CSchemeRegister::CSchemeRegister(SchemeID ID, const char* pName, tURIAttributions attr)
{
    CUriManager* pUriMgr = CUriManager::Instance();
    ASSERT(pUriMgr);
    bool bRes = pUriMgr->Register(ID, pName, attr);
    ASSERT(bRes);
}

CSchemeRegister::~CSchemeRegister()
{
}
