/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __COOKIEAPP_COOKIE_CMD_HANDLER_H__
#define __COOKIEAPP_COOKIE_CMD_HANDLER_H__

#include "CmdLine/CmdHandler.h"
#include "HTTP/HttpCookieData.h"

class CHttpCookieDB;
class CCookieCmdHandler : public CCmdHandler
{
public:
    CCookieCmdHandler(CHttpCookieDB& db) :
        CCmdHandler(), m_CookieDB(db) {}
    ~CCookieCmdHandler() {}

    // From CCmdHandler
    bool HandleCommand(CLineArguments* pArgs);

private:
    static void DumpCookie(
        void* pData, const char* pDomain, CHttpCookieData::Attribute* pAttr);

private:
    CHttpCookieDB& m_CookieDB;
};

#endif