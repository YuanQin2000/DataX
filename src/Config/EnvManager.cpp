/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "EnvManager.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "Common/Macros.h"
#include "Tracker/Trace.h"

using std::sprintf;
using std::malloc;
using std::free;
using std::strerror;
using std::strlen;
using std::getenv;

const char* CEnvManager::s_pRootPathVariableName = "BYTEC_ROOT_PATH";
const char* CEnvManager::s_pConfigPathVariableName = "BYTEC_CONFIG_PATH";

CEnvManager::CEnvManager() :
    m_pRootWorkPath(NULL),
    m_pHttpWorkPath(NULL),
    m_pConfigPath(NULL),
    m_RootWorkPathLength(0),
    m_HttpWorkPathLength(0),
    m_ConfigPathLength(0),
    m_bOwnedConfigPath(false)
{
    int step = 0;
    const char* pRootPath = getenv(s_pRootPathVariableName);
    const char* pConfigPath = getenv(s_pConfigPathVariableName);
    OUTPUT_DEBUG_TRACE("Environment Set:\n\tRoot Path: %s\n\tConfig Path: %s\n", pRootPath, pConfigPath);
    if (pRootPath) {
        if (!IsValidPath(pRootPath)) {
            step = 1;
            goto FAILED_EXIT;
        }
    }
    if (!SetRootWorkPath(pRootPath)) {
        step = 2;
        goto FAILED_EXIT;
    }
    if (pConfigPath) {
        if (!IsValidPath(pConfigPath)) {
            step = 3;
            goto FAILED_EXIT;
        }
        if (!SetConfigPath(pConfigPath, false)) {
            step = 4;
            goto FAILED_EXIT;
        }
    } else {
        if (!SetConfigPath(m_pRootWorkPath, true)) {
            step = 5;
            goto FAILED_EXIT;
        }
    }
    
    return;

FAILED_EXIT:
    Release();
    OUTPUT_ERROR_TRACE("Build running environment failed, step: %d.\n", step);
    EXIT_PROGRAM(1);
}

CEnvManager::~CEnvManager()
{
    Release();
}

bool CEnvManager::SetRootWorkPath(const char* pDirName)
{
    bool bRes = false;
    m_pRootWorkPath = CreateDirectory(pDirName, NULL);
    if (m_pRootWorkPath) {
        m_RootWorkPathLength = strlen(m_pRootWorkPath);
        bRes = SetAllSubWorkPath();
    }
    return bRes;
}

bool CEnvManager::SetConfigPath(const char* pDirName, bool bSubConfigDir)
{
    static const char* s_pSubConfigDirName = "config";
    static const size_t s_SubConfigDirNameLen = strlen(s_pSubConfigDirName);

    bool bRes = false;
    size_t dirLen = strlen(pDirName);
    m_bOwnedConfigPath = bSubConfigDir;
    if (bSubConfigDir) {
        if (pDirName[dirLen - 1] == '/') {
            if (dirLen > 1) {
                char* pConfigPath = reinterpret_cast<char*>(
                    malloc(dirLen + s_SubConfigDirNameLen + 1));
                if (pConfigPath) {
                    sprintf(pConfigPath, "%s%s", pDirName, s_pSubConfigDirName);
                    m_pConfigPath = pConfigPath;
                    m_ConfigPathLength = dirLen + s_SubConfigDirNameLen;
                    bRes = true;
                }
            }
        } else {
            char* pConfigPath = reinterpret_cast<char*>(
                malloc(dirLen + s_SubConfigDirNameLen + 2));
            if (pConfigPath) {
                sprintf(pConfigPath, "%s/%s", pDirName, s_pSubConfigDirName);
                m_pConfigPath = pConfigPath;
                m_ConfigPathLength = dirLen + s_SubConfigDirNameLen + 1;
                bRes = true;
            }
        }
    } else {
        m_pConfigPath = pDirName;
        m_ConfigPathLength = dirLen;
        bRes = true;
    }
    return bRes;
}

bool CEnvManager::SetAllSubWorkPath()
{
    // HTTP work path
    m_pHttpWorkPath = CreateDirectory("HTTP", "../");
    if (m_pHttpWorkPath == NULL) {
        return false;
    }
    m_HttpWorkPathLength = strlen(m_pHttpWorkPath);

    return true;
}

char* CEnvManager::CreateDirectory(
    const char* pDirName, const char* pEnterDirAfterFinished)
{
    char* pCreatedPath = NULL;
    if (pDirName != NULL) {
        if (chdir(pDirName) != 0) {
            if (errno != ENOENT) {
                OUTPUT_ERROR_TRACE("chdir %s: %s\n", pDirName, strerror(errno));
                return NULL;
            }
            if (mkdir(pDirName, 0755) != 0) {
                OUTPUT_ERROR_TRACE("mkdir %s: %s\n", pDirName, strerror(errno));
                return NULL;
            }
            if (chdir(pDirName) != 0) {
                OUTPUT_ERROR_TRACE("chdir %s: %s\n", pDirName, strerror(errno));
                return NULL;
            }
        }
    }
    pCreatedPath = get_current_dir_name();
    if (pEnterDirAfterFinished) {
        if (chdir(pEnterDirAfterFinished) != 0) {
            OUTPUT_ERROR_TRACE("chdir %s: %s\n", pEnterDirAfterFinished, strerror(errno));
            free(pCreatedPath);
            pCreatedPath = NULL;
        }
    }
    return pCreatedPath;
}

void CEnvManager::Release()
{
    if (m_pRootWorkPath) {
        free(m_pRootWorkPath);
        m_pRootWorkPath = NULL;
    }
    if (m_pHttpWorkPath) {
        free(m_pHttpWorkPath);
        m_pHttpWorkPath = NULL;
    }
    if (m_bOwnedConfigPath && m_pConfigPath) {
        free(const_cast<char*>(m_pConfigPath));
        m_pConfigPath = NULL;
    }
}

bool CEnvManager::IsValidPath(const char* pPath)
{
    // Must be absolute path and not root path.
    return pPath[0] == '/' && pPath[1] != '\0';
}
