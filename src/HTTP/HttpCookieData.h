/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_COOKIE_DEFS_H__
#define __HTTP_COOKIE_DEFS_H__

#include <cstdlib>
#include <cstring>
#include "Common/Typedefs.h"
#include "Common/IntrusiveFwList.h"
#include "DataBase/KeyValueDB.h"
#include "Tracker/Trace.h"

using std::free;

#define COOKIE_DEFAULT_MAX_AGE 20 * 60 // 20 minutes

struct HttpCookieValue {
    char* pName;
    char* pValue;

    HttpCookieValue(char* pK, char* pV) : pName(pK), pValue(pV) {}
};

class CByteData;
class CHttpCookieData
{
public:
    struct Attribute {
        const char* pPath;
        const char* pName;
        const char* pValue;
        tSecondTick ExpireTime;
        bool bSecure;
        bool bWildcardMatched;

        Attribute(
            const char* pP = NULL,
            const char* pN = NULL,
            const char* pV = NULL,
            tSecondTick expire = 0,
            bool bSec = false,
            bool bSub = false) :
            pPath(pP),
            pName(pN),
            pValue(pV),
            ExpireTime(expire),
            bSecure(bSec),
            bWildcardMatched(bSub) {}

        void Dump()
        {
            OUTPUT_RAW_TRACE(
                "\tName: %s\n\tValue: %s\n"
                "\tPath: %s\n\tExpire: %ld\n"
                "\tSecure: %s\n\tWild Matched: %s\n",
                pName, pValue, pPath, ExpireTime,
                bSecure ? "TRUE" : "FALSE",
                bWildcardMatched ? "TRUE" : "FALSE");
        }
    };

    CHttpCookieData()
    {
        m_pHostName = NULL;
        m_AttrData.pPath = NULL;
        m_AttrData.pName = NULL;
        m_AttrData.pValue = NULL;
        m_AttrData.ExpireTime = 0;
        m_AttrData.bSecure = false;
        m_AttrData.bWildcardMatched = false;
    }

    CHttpCookieData(
        char* pDomain,
        const char* pP,
        const char* pN,
        const char* pV,
        tSecondTick expire = 0,
        bool bSec = false,
        bool bWild = false) :
        m_pHostName(pDomain),
        m_AttrData(pP, pN, pV, expire, bSec, bWild) {}

    void Set(
        char* pDomain,
        const char* pP,
        const char* pN,
        const char* pV,
        tSecondTick expire,
        bool bSec,
        bool bWild)
    {
        m_pHostName = pDomain;
        m_AttrData.pPath = pP;
        m_AttrData.pName = pN;
        m_AttrData.pValue = pV;
        m_AttrData.ExpireTime = expire;
        m_AttrData.bSecure = bSec;
        m_AttrData.bWildcardMatched = bWild;
    }

    const Attribute* GetAttribute() const { return &m_AttrData; }
    Attribute* GetAttribute() { return &m_AttrData; }
    const char* GetDomainName() const { return m_pHostName; }

    static CByteData* Serialize(Attribute* pCookieAttr, CByteData* pOutData);
    static bool UnSerialize(CByteData* pData, Attribute* pOutCookieAttr);

    static CKeyValueDB::tRecordCompareFunc GetCookieCompareFunc()
    {
        return reinterpret_cast<CKeyValueDB::tRecordCompareFunc>(std::strcmp);
    }

    void Dump()
    {
        OUTPUT_RAW_TRACE("\tDomain: %s\n", m_pHostName);
        m_AttrData.Dump();
    }

private:
    char* m_pHostName;
    Attribute m_AttrData;

    static const uint8_t WILD_CARD_FLAG = 0x01;  //00000001
    static const uint8_t SECURE_FLAG = 0x02;  //00000010
};

#endif