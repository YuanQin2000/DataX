/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __URI_MANAGER_H__
#define __URI_MANAGER_H__

#include <map>
#include <cstring>

#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "SchemeDefines.h"
#include "URI.h"
#include "Common/Singleton.h"
#include "Thread/Lock.h"

using std::map;


class CSchemeRegister;
class CUriManager : public CSingleton<CUriManager>
{
public:
    bool GetSchemeID(const char* pString, SchemeID& ID);
    bool GetAttribution(const char* pString, tURIAttributions& attr);

    const char* SchemeID2String(SchemeID ID)
    {
        ASSERT(ID >= 0 && ID < SCHEME_COUNT);
        ASSERT(IsRegistered(ID));
        return m_SchemeNames[ID];
    }

    tURIAttributions Attribution(SchemeID ID)
    {
        ASSERT(ID >= 0 && ID < SCHEME_COUNT);
        ASSERT(IsRegistered(ID));
        return m_Attributions[ID];
    }

    bool IsRegistered(SchemeID ID)
    {
        return m_SchemeNames[ID] != NULL;
    }

private:
    CUriManager();
    ~CUriManager();

    bool Register(SchemeID ID, const char* pName, tURIAttributions attr);

private:
    CCriticalSection m_CS;
    map<const char*, SchemeID, tStringCompareFunc> m_RegisteredScheme;
    tURIAttributions m_Attributions[SCHEME_COUNT];
    const char*      m_SchemeNames[SCHEME_COUNT];

    friend class CSingleton<CUriManager>;
    friend class CSchemeRegister;
};


class CSchemeRegister
{
public:
    CSchemeRegister(SchemeID ID, const char* pName, tURIAttributions attr);
    ~CSchemeRegister();

    DISALLOW_COPY_CONSTRUCTOR(CSchemeRegister);
    DISALLOW_ASSIGN_OPERATOR(CSchemeRegister);
    DISALLOW_DEFAULT_CONSTRUCTOR(CSchemeRegister);
};

#endif