/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_TOKEN_H__
#define __HTTP_BASE_TOKEN_H__

#include "Common/Typedefs.h"
#include "Common/Singleton.h"
#include "Thread/Lock.h"

#define INVALID_TOKEN_ID  -1
#define INVALID_TOKEN_CATEGORY -1

typedef int tTokenCategory;
typedef int tTokenID;


class ITokenMap
{
public:
    virtual const char* GetTokenString(
        tTokenCategory categoryID, tTokenID tokenID) const = 0;
    virtual tTokenID GetTokenID(
        tTokenCategory categoryID, const char* pString, size_t len) const = 0;
    virtual bool CheckValidity(tTokenCategory categoryID, tTokenID tokenID) const = 0;
};


class CTokenMapRegister;
class CTokenManager : public CSingleton<CTokenManager>
{
public:
    ITokenMap* GetTokenMapInstance() const { return m_pTokenMap; }

    tTokenID GetTokenID(
        tTokenCategory categoryID, const char* pString, size_t len) const
    {
        return m_pTokenMap->GetTokenID(categoryID, pString, len);
    }

    const char* GetTokenString(tTokenCategory categoryID, tTokenID tokenID) const
    {
        return m_pTokenMap->GetTokenString(categoryID, tokenID);
    }

    bool CheckValidity(tTokenCategory categoryID, tTokenID tokenID) const
    {
        return m_pTokenMap->CheckValidity(categoryID, tokenID);
    }

protected:
    CTokenManager() :
        m_CS(),
        m_pTokenMap(NULL) {}

    ~CTokenManager() {}

private:
    bool RegisterTokenMap(ITokenMap* pInstance);

private:
    CCriticalSection m_CS;

    ITokenMap* m_pTokenMap;

    friend class CTokenMapRegister;
    friend class CSingleton<CTokenManager>;
};


class CTokenMapRegister
{
public:
    CTokenMapRegister(ITokenMap* pInstance);
    ~CTokenMapRegister() {}

    DISALLOW_COPY_CONSTRUCTOR(CTokenMapRegister);
    DISALLOW_ASSIGN_OPERATOR(CTokenMapRegister);
    DISALLOW_DEFAULT_CONSTRUCTOR(CTokenMapRegister);
};


#endif