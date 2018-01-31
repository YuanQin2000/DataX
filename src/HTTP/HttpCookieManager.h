/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_COOKIE_MANAGER_H__
#define __HTTP_COOKIE_MANAGER_H__

#include "HttpCookieData.h"
#include "HttpCookieDB.h"
#include "Common/Typedefs.h"
#include "Common/ForwardList.h"
#include "URI/URI.h"
#include "Network/Domain.h"
#include "Tracker/Trace.h"

class CHttpCookieValueList
{
public:
    CHttpCookieValueList(CMemory* pMemAllocator = CMemory::GetDefaultMemory()) :
        m_List(pMemAllocator),
        m_pMemAllocator(pMemAllocator) {}
    ~CHttpCookieValueList() { Reset(); }

    void Reset()
    {
        ReleaseCookies();
        m_List.Reset();
    }

    bool Append(const char* pName, const char* pValue);

    CForwardList& GetList() { return m_List; }

private:
    void ReleaseCookies();

private:
    CForwardList m_List;
    CMemory* m_pMemAllocator;
};


class CByteData;
class CHttpCookieManager
{
public:
    CHttpCookieManager(CHttpCookieDB* pDB) : m_pCookieDB(pDB)
    {
        ASSERT(pDB);
    }

    ~CHttpCookieManager()
    {
        delete m_pCookieDB;
    }

    bool SetCookie(CUri* pURI, CHttpCookieData* pItem);
    bool GetCookies(CUri* pURI, CHttpCookieValueList* pList);

    static CHttpCookieManager* GetDefaultInstance();
    static CHttpCookieManager* CreateInstance(const char* pPath, const char* pDBFileName);
    static CHttpCookieDB* CreateCookieDB(const char* pPath, const char* pDBFileName);

private:
    struct HandlerData {
        bool bSecure;
        const char* pPath;
        CHttpCookieValueList* pOutList;

        HandlerData(
            bool secure,
            const char* path,
            CHttpCookieValueList* list) :
            bSecure(secure),
            pPath(path),
            pOutList(list) {}
    };

    bool ProcessCookieDomain(
        const char* pRequestedDomain,
        const char* pCookieDomain,
        NSInternetDomain::DomainDesc* pOutDesc);

    static void HandleMatchedCookie(
        void* pData, const char* pDomain, CHttpCookieData::Attribute* pAttr);
    static bool IsSubOrEqualPath(const char* pPath1, const char* pPath2)
    {
        CUri::PathRelation relation = CUri::PathCompare(pPath1, pPath2);
        return relation == CUri::PathMatched || relation == CUri::PathIsChild;
    }

private:
    CHttpCookieDB* m_pCookieDB; // Owned

    DISALLOW_COPY_CONSTRUCTOR(CHttpCookieManager);
    DISALLOW_ASSIGN_OPERATOR(CHttpCookieManager);
};

#endif