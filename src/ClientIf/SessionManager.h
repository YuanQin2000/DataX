/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_IF_SESSION_MANAGER_H__
#define __CLIENT_IF_SESSION_MANAGER_H__

#include "Common/Singleton.h"

class CSessionManager : public CSingleton<CSessionManager>
{
public:
    // __sync_add_and_fetch
    uint16_t NewSesssion()          { return ++m_CurrentSession; }
    uint16_t CurrentSession() const { return m_CurrentSession; }

protected:
    CSessionManager() : m_CurrentSession(0) {}
    ~CSessionManager() {}

private:
    uint16_t m_CurrentSession;

    friend class CSingleton<CSessionManager>;
};
#endif