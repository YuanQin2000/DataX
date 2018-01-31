/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpCookieDB.h"
#include "Common/ByteData.h"
#include "HTTPBase/DateHelper.h"
#include "DataBase/KeyValueDB.h"
#include "Network/Domain.h"
#include "Tracker/Trace.h"

bool CHttpCookieDB::StoreCookie(
    CHttpCookieData::Attribute* pAttr, const char* pDomain, size_t domainLen /* = 0 */)
{
    CByteData value;
    if (!CHttpCookieData::Serialize(pAttr, &value)) {
        OUTPUT_WARNING_TRACE("Can not serialize cookie data.\n");
        return false;
    }

    bool bRes = false;
    size_t dataLen = domainLen == 0 ? strlen(pDomain) + 1 : domainLen + 1;
    const CByteData key(const_cast<char*>(pDomain), dataLen);
    CKeyValueDB::Iterator* pIterator =
        m_pDBEngine->CreateIterator(CKeyValueDB::Iterator::NORMAL, &key, &value);
    if (pIterator) {
        bRes = pIterator->IsEnd() ?
            m_pDBEngine->SetValue(&key, &value) :
            pIterator->UpdateValue(&value);
        delete pIterator;
    }
    return bRes;
}

bool CHttpCookieDB::QueryCookies(
    const char* pDomain,
    tCookieDataHandler handler,
    void* pData,
    bool bHandleAll,
    bool bDeleteExpired)
{
    ASSERT(handler);

    HandlerData handlerData(handler, pData, bHandleAll, bDeleteExpired, true);
    if (pDomain) {
        NSInternetDomain::CDomainSectionAccess access(pDomain);
        const char* pRegisteredDomain = access.RegisteredDomain();
        if (pRegisteredDomain == NULL) {
            return false;
        }

        for (const char* pCurrent = access.Begin();
            pCurrent && pCurrent <= pRegisteredDomain;
            pCurrent = access.Next()) {
            const CByteData key(const_cast<char*>(pCurrent), access.CurrentLength() + 1);
            if (!m_pDBEngine->GetAllRecords(&key, HandleCookieData, &handlerData)) {
                return false;
            }
            handlerData.bDomainExactMatched = false;
        }
        return true;
    }

    // else pDomain == NULL
    return m_pDBEngine->GetAllRecords(NULL, HandleCookieData, &handlerData);
}

CKeyValueDB::ActionOnRecord
CHttpCookieDB::HandleCookieData(
    CKeyValueDB::Iterator* pRecord, void* pData)
{
    ASSERT(pData);

    CKeyValueDB::ActionOnRecord action = CKeyValueDB::ACTION_NONE;
    HandlerData* pHandlerData = reinterpret_cast<HandlerData*>(pData);
    CHttpCookieData::Attribute attr;
    if (CHttpCookieData::UnSerialize(pRecord->Value(), &attr)) {
        bool bExpired = CDateHelper::IsExpired(attr.ExpireTime);
        bool bCallClient = pHandlerData->bSendAllToClient || !bExpired;
        if (bCallClient && (pHandlerData->bDomainExactMatched || attr.bWildcardMatched)) {
            pHandlerData->pClientHandler(
                pHandlerData->pClientData, reinterpret_cast<char*>(pRecord->Key()->GetData()), &attr);
        }
        if (bExpired && pHandlerData->bDeleteExpired) {
            pRecord->EraseRecord();
            action = CKeyValueDB::ACTION_DELETED;
        }
    } else {
        pRecord->EraseRecord();
        action = CKeyValueDB::ACTION_DELETED;
        OUTPUT_WARNING_TRACE("Corrupted cookie data, delete...\n");
    }
    return action;
}
