/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TLS_CERT_MANAGER_H__
#define __TLS_CERT_MANAGER_H__

#include "Common/Singleton.h"

class CX509Cert;
class CCertManager : public CSingleton<CCertManager>
{
public:
    const char* RootCA() const { return m_pRootCA; }
    const char* MyCert() const { return m_pMyCert; }
    const char* MyPrivateKey() const { return m_pMyPrivateKey; }

    static CX509Cert* GetCertByFileName(const char* pCertName);

protected:
    CCertManager();
    ~CCertManager() {}

private:
    const char* m_pRootCA;
    const char* m_pMyCert;
    const char* m_pMyPrivateKey;

    friend class CSingleton<CCertManager>;
};

#endif