/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include "HTTP/HttpCookieDB.h"
#include "HTTP/HttpCookieManager.h"
#include "Common/Typedefs.h"
#include "Common/ByteData.h"
#include "CmdLine/CommandLine.h"
#include "CmdLine/CommonCmdHandler.h"
#include "CookieCmdHandler.h"

using std::vector;

static bool CreateCommandHandlers(
    vector<CCmdHandler*>& handlers, CHttpCookieDB& db)
{
    CCmdHandler* pBuiltinCmdHandler = new CCommonCmdHandler();
    CCmdHandler* pCookieCmdHandler = new CCookieCmdHandler(db);
    if (pBuiltinCmdHandler != NULL && pCookieCmdHandler != NULL) {
        handlers.push_back(pBuiltinCmdHandler);
        handlers.push_back(pCookieCmdHandler);
        return true;
    }
    delete pCookieCmdHandler;
    delete pBuiltinCmdHandler;
    return false;
}

static void DestroyCommandHandlers(vector<CCmdHandler*>& handlers)
{
    vector<CCmdHandler*>::iterator iter = handlers.begin();
    vector<CCmdHandler*>::iterator iterEnd = handlers.end();
    while (iter != iterEnd) {
        delete *iter;
        ++iter;
    }
}

static bool ParsePath(
    const char* pPath, char** pOutBaseName, char** pOutFileName)
{
    const char* pSlash = NULL;
    const char* pCur = pPath;
    char ch = *pCur;
    while (ch != '\0') {
        if (ch == '/') {
            pSlash = pCur;
        }
        ch = *++pCur;
    }
    if (pSlash == NULL) {
        // Use current directory.
        *pOutFileName = strdup(pPath);
        *pOutBaseName = get_current_dir_name();
        return true;
    }

    size_t dirLen = pSlash - pPath;
    char* pDir = NULL;
    if (*pPath != '/') {
        // Relative path.
        const char* pCurPath = getenv("PWD");
        size_t len = 0;
        if (pCurPath && (len = strlen(pCurPath)) > 0) {
            pDir = reinterpret_cast<char*>(malloc(len + dirLen + 2));
            if (pDir) {
                memcpy(pDir, pCurPath, len);
                pDir[len] = '/';
                memcpy(pDir + len + 1, pPath, dirLen);
                pDir[len + 1 + dirLen] = '\0';
            }
        }
    } else {
        // Absolute path.
        pDir = reinterpret_cast<char*>(malloc(dirLen + 1));
        if (pDir) {
            memcpy(pDir, pPath, dirLen);
            pDir[dirLen] = '\0';
        }
    }
    *pOutFileName = strdup(pSlash + 1);
    *pOutBaseName = pDir;
    return true;
}

static void Usage(const char* pProgName)
{
    printf("Usage: %s -d cookie-db\n", pProgName);
}

int main(int argc, char* argv[])
{
    int ch = 0;
    const char* pDBFileString = NULL;
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            pDBFileString = optarg;
            break;
        default:
            Usage(argv[0]);
            return -1;
            break;
        }
    }
    if (pDBFileString == NULL) {
        Usage(argv[0]);
        return -1;
    }

    struct stat statData;
    if (stat(pDBFileString, &statData) == -1) {
        printf("stat: %s: %s\n", pDBFileString, strerror(errno));
        return -1;
    }
    if (!S_ISREG(statData.st_mode)) {
        printf("DB file %s is not regular file\n", pDBFileString);
        return -1;
    }

    char* pWorkPath = NULL;
    char* pDBFileName = NULL;
    if (!ParsePath(pDBFileString, &pWorkPath, &pDBFileName)) {
        return -1;
    }

    int res = -1;
    CHttpCookieDB* pCookieDB = CHttpCookieManager::CreateCookieDB(pWorkPath, pDBFileName);
    if (pCookieDB) {
        vector<CCmdHandler*> cmdHandlers;
        CCommandLine* pCmdline = NULL;
        if (!CreateCommandHandlers(cmdHandlers, *pCookieDB)) {
            printf("Create Command Handlers Failed\n");
            goto EXIT;
        }

        pCmdline = new CCommandLine(cmdHandlers, "> ");
        if (pCmdline == NULL) {
            printf("Create Command Line Failed\n");
            goto EXIT;
        }
        res = pCmdline->Loop();
        delete pCmdline;
        DestroyCommandHandlers(cmdHandlers);
    }

EXIT:
    delete pCookieDB;
    free(pWorkPath);
    free(pDBFileName);
    return res;
}
