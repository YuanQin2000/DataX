/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpCookieManager.h"
#include <cstring>
#include <cstdlib>
#include <new>
#include "HttpCookieDB.h"
#include "DataBase/KeyValueDB.h"
#include "Common/Macros.h"
#include "Common/ByteData.h"
#include "Thread/Lock.h"
#include "URI/URI.h"
#include "Tracker/Trace.h"
#include "Config/EnvManager.h"

using std::malloc;
using std::free;
using std::memcpy;

bool CHttpCookieValueList::Append(const char* pName, const char* pValue)
{
    char* pDupName = NULL;
    char* pDupValue = NULL;
    HttpCookieValue* pCookieValue = NULL;

    size_t nameLen = strlen(pName) + 1;
    size_t valueLen = strlen(pValue) + 1;
    if (nameLen == 1 || valueLen == 1) {
        return false;
    }

    size_t memSize = sizeof(HttpCookieValue) + nameLen + valueLen;
    char* pMem = reinterpret_cast<char*>(m_pMemAllocator->Malloc(memSize));
    if (pMem) {
        pDupName = pMem + sizeof(HttpCookieValue);
        pDupValue = pDupName + nameLen;
        pCookieValue = new (pMem) HttpCookieValue(pDupName, pDupValue);
    }

    if (pCookieValue) {
        memcpy(pDupName, pName, nameLen);
        memcpy(pDupValue, pValue, valueLen);
        m_List.PushBack(pCookieValue);
        return true;
    }
    return false;
}

void CHttpCookieValueList::ReleaseCookies()
{
    CForwardList::Iterator iter = m_List.Begin();
    CForwardList::Iterator iterEnd = m_List.End();
    while (iter != iterEnd) {
        void* pMem = m_List.DataAt(iter);
        ASSERT(pMem);
        m_pMemAllocator->Free(pMem);
        ++iter;
    }
}

CHttpCookieManager* CHttpCookieManager::GetDefaultInstance()
{
    static CHttpCookieManager* s_pInstance = NULL;
    static CCriticalSection s_CS;

    if (s_pInstance) {
        return s_pInstance;
    }
    s_CS.Lock();
    if (s_pInstance == NULL) {
        s_pInstance = CreateInstance(
            CEnvManager::Instance()->HttpWorkPath(), "cookie.db");
    }
    s_CS.Unlock();
    return s_pInstance;
}

CHttpCookieManager*
CHttpCookieManager::CreateInstance(const char* pPath, const char* pDBFileName)
{
    CHttpCookieManager* pInstance = NULL;
    CHttpCookieDB* pDB = CHttpCookieManager::CreateCookieDB(pPath, pDBFileName);
    if (pDB) {
        pInstance = new CHttpCookieManager(pDB);
        if (pInstance == NULL) {
            delete pDB;
        }
    }
    return pInstance;
}

CHttpCookieDB* CHttpCookieManager::CreateCookieDB(const char* pPath, const char* pDBFileName)
{
    CKeyValueDB* pDBEngine = CKeyValueDB::CreateInstance(
        pPath, pDBFileName, CHttpCookieData::GetCookieCompareFunc(), true);
    CHttpCookieDB* pInstance = NULL;
    if (pDBEngine) {
        pInstance = new CHttpCookieDB(pDBEngine);
        if (pInstance == NULL) {
            delete pDBEngine;
        }
    }
    return pInstance;
}

bool CHttpCookieManager::SetCookie(CUri* pURI, CHttpCookieData* pItem)
{
    TRACK_FUNCTION_LIFE_CYCLE;
    ASSERT(pURI);
    ASSERT(pItem);

    CHttpCookieData::Attribute* pAttr = pItem->GetAttribute();
    if (pAttr->pName == NULL || pAttr->pValue == NULL) {
        OUTPUT_WARNING_TRACE("Empty Name or Value.\n");
        return false;
    }
    if (pURI->Authority()->HostName().bIsIP) {
        OUTPUT_NOTICE_TRACE(
            "Disable Cookie for IP host: %s\n", pURI->Authority()->HostName().pName);
        return false;
    }

    if (pAttr->pPath == NULL) {
        pAttr->pPath = pURI->Path();
        if (pAttr->pPath == NULL) {
            pAttr->pPath = "/";
        }
    } else if (!IsSubOrEqualPath(pURI->Path(), pAttr->pPath)) {
        OUTPUT_NOTICE_TRACE(
            "The Set-Cookie Path is not equal or super to the URI target path: %s, %s\n",
            pAttr->pPath, pURI->Path());
        return false;
    }

    bool bRes = false;
    NSInternetDomain::DomainDesc desc;
    if (ProcessCookieDomain(pURI->Authority()->HostName().pName, pItem->GetDomainName(), &desc)) {
        pAttr->bWildcardMatched = desc.bWildCard;
        bRes = m_pCookieDB->StoreCookie(pAttr, desc.pHostDomainName, desc.Length);
    }
    return bRes;
}

bool CHttpCookieManager::GetCookies(CUri* pURI, CHttpCookieValueList* pList)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    ASSERT(pURI);
    ASSERT(pList);

    const char* pDomainStr = pURI->Authority()->HostName().pName;
    if (pURI->Authority()->HostName().bIsIP) {
        return false;
    }

    bool bSecure = (pURI->Scheme() == SCHEME_HTTPS);
    HandlerData handlerData(bSecure, pURI->Path(), pList);
    return m_pCookieDB->QueryCookies(
        pDomainStr, HandleMatchedCookie, &handlerData, false, true);
}

bool CHttpCookieManager::ProcessCookieDomain(
    const char* pRequestedDomain,
    const char* pCookieDomain,
    NSInternetDomain::DomainDesc* pOutDesc)
{
    if (pCookieDomain == NULL) {
        return NSInternetDomain::ParseString(pRequestedDomain, pOutDesc);
    }

    bool bRes = false;
    if (NSInternetDomain::ParseString(pCookieDomain, pOutDesc)) {
        if (NSInternetDomain::IsMatch(pOutDesc, pRequestedDomain)) {
            bRes = true;
        }
    }
    return bRes;
}

void CHttpCookieManager::HandleMatchedCookie(
    void* pData, const char* pDomain, CHttpCookieData::Attribute* pAttr)
{
    HandlerData* pHandlerData = reinterpret_cast<HandlerData*>(pData);
    if (pAttr->bSecure == pHandlerData->bSecure &&
        IsSubOrEqualPath(pHandlerData->pPath, pAttr->pPath)) {
        // Cookie matched.
        pHandlerData->pOutList->Append(pAttr->pName, pAttr->pValue);
    }
}
