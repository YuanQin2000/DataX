/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_COOKIE_DB_H__
#define __HTTP_COOKIE_DB_H__

#include "HttpCookieData.h"
#include "Common/Typedefs.h"
#include "Common/ByteData.h"
#include "Tracker/Trace.h"

class CKeyValueDB;
class CHttpCookieDB
{
public:
    typedef void (*tCookieDataHandler)(
        void* pData, const char* pDomain, CHttpCookieData::Attribute* pAttr);

    CHttpCookieDB(CKeyValueDB* pDBEngine) : m_pDBEngine(pDBEngine)
    {
        ASSERT(pDBEngine);
    }

    ~CHttpCookieDB()
    {
        delete m_pDBEngine;
    }

    bool StoreCookie(
        CHttpCookieData::Attribute* pAttr,
        const char* pDomain,
        size_t domainLen = 0);
    bool QueryCookies(
        const char* pDomain,
        tCookieDataHandler handler,
        void* pData,
        bool bHandleAll,
        bool bDeletedExpired);

private:
    struct HandlerData {
        tCookieDataHandler pClientHandler;
        void* pClientData;
        bool bSendAllToClient;
        bool bDeleteExpired;
        bool bDomainExactMatched;

        HandlerData(
            tCookieDataHandler handler,
            void* pData,
            bool bSendAll,
            bool bDeleteExpr,
            bool bExactMatch) :
            pClientHandler(handler),
            pClientData(pData),
            bSendAllToClient(bSendAll),
            bDeleteExpired(bDeleteExpr),
            bDomainExactMatched(bExactMatch) {}
    };

    static CKeyValueDB::ActionOnRecord HandleCookieData(
        CKeyValueDB::Iterator* pRecord, void* pData);

private:
    CKeyValueDB* m_pDBEngine;   // Owned

    DISALLOW_DEFAULT_CONSTRUCTOR(CHttpCookieDB);
    DISALLOW_COPY_CONSTRUCTOR(CHttpCookieDB);
    DISALLOW_ASSIGN_OPERATOR(CHttpCookieDB);
};

#endif