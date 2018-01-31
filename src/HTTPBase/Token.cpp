/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Token.h"

#include "Tracker/Trace.h"

bool CTokenManager::RegisterTokenMap(ITokenMap* pInstance)
{
    ASSERT(pInstance);

    CSectionLock lock(m_CS);
    ASSERT(!m_pTokenMap);
    m_pTokenMap = pInstance;
    return true;
}

CTokenMapRegister::CTokenMapRegister(ITokenMap* pInstance)
{
    CTokenManager::Instance()->RegisterTokenMap(pInstance);
}
