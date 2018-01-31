/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CookieCmdHandler.h"
#include <stdio.h>
#include "CmdLine/CommandLine.h"
#include "CmdLine/LineArguments.h"
#include "HTTP/HttpCookieDB.h"
#include "HTTPBase/DateHelper.h"

bool CCookieCmdHandler::HandleCommand(CLineArguments* pArgs)
{
    bool bRes = false;
    CLineArguments::ArgumentData* pParam = pArgs->GetArgumentAt(0);
    size_t argCount = pArgs->ArgumentsCount();
    if (pParam->Type == CLineArguments::AT_STRING && strcmp(pParam->pString, "dump") == 0) {
        const char* pDomainStr = NULL;
        if (argCount == 1) {
            bRes = true;
        } else if (argCount == 2) {
            CLineArguments::ArgumentData* pParam1 = pArgs->GetArgumentAt(1);
            if (pParam1->Type == CLineArguments::AT_STRING) {
                pDomainStr = pParam1->pString;
                bRes = true;
            }
        }
        if (bRes) {
            m_CookieDB.QueryCookies(pDomainStr, DumpCookie, NULL, true, false);
        }
    }
    return bRes;
}

void CCookieCmdHandler::DumpCookie(
    void* pData, const char* pDomain, CHttpCookieData::Attribute* pAttr)
{
    char timeStrBuffer[64];
    const char* pDomainDesc = "";
    if (pAttr->bSecure) {
        pDomainDesc = "(Secure)";
    }
    size_t timeStrLen = 0;
    CDateHelper::Serialize(
        timeStrBuffer, sizeof(timeStrBuffer) - 1, pAttr->ExpireTime, &timeStrLen);
    timeStrBuffer[timeStrLen] = '\0';
    if (pAttr->bWildcardMatched) {
        printf(".%s%s:\n\tName: %s\n\tValue: %s\n\tPath: %s\n\tExpire: %s\n",
            pDomain, pDomainDesc, pAttr->pName, pAttr->pValue, pAttr->pPath, timeStrBuffer);
    } else {
        printf("%s%s:\n\tName: %s\n\tValue: %s\n\tPath: %s\n\tExpire: %s\n",
            pDomain, pDomainDesc, pAttr->pName, pAttr->pValue, pAttr->pPath, timeStrBuffer);
    }
}
