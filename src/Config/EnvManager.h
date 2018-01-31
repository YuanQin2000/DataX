/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CORE_ENV_MANAGER_H__
#define __CORE_ENV_MANAGER_H__

#include "Common/Limits.h"
#include "Common/Singleton.h"
#include "Tracker/Trace.h"

class CEnvManager : public CSingleton<CEnvManager>
{
public:
    ~CEnvManager();

    const char* RootWorkPath()
    {
        ASSERT(m_pRootWorkPath);
        return m_pRootWorkPath;
    }
    const char* HttpWorkPath()
    {
        ASSERT(m_pHttpWorkPath);
        return m_pHttpWorkPath;
    }
    const char* ConfigPath()
    {
        ASSERT(m_pConfigPath);
        return m_pConfigPath;
    }

    size_t RootWorkPathLength()
    {
        ASSERT(m_RootWorkPathLength != 0);
        return m_RootWorkPathLength;
    }
    size_t HttpWorkPathLength()
    {
        ASSERT(m_HttpWorkPathLength != 0);
        return m_HttpWorkPathLength;
    }
    size_t ConfigPathLength()
    {
        ASSERT(m_ConfigPathLength != 0);
        return m_ConfigPathLength;
    }

protected:
    CEnvManager();

private:
    bool SetRootWorkPath(const char* pDirName);
    bool SetConfigPath(const char* pDirName, bool bSubConfigDir);
    bool SetAllSubWorkPath();

    void Release();

    static char* CreateDirectory(
        const char* pDirName, const char* pEnterDirAfterFinished);
    static bool IsValidPath(const char* pPath);

private:
    char* m_pRootWorkPath;
    char* m_pHttpWorkPath;
    const char* m_pConfigPath;
    size_t m_RootWorkPathLength;
    size_t m_HttpWorkPathLength;
    size_t m_ConfigPathLength;
    bool m_bOwnedConfigPath;

    static const char* s_pRootPathVariableName;
    static const char* s_pConfigPathVariableName;
    friend class CSingleton<CEnvManager>;
};

#endif
