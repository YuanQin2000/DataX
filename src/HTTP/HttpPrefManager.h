/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_PREF_MANAGER_H__
#define __HTTP_PREF_MANAGER_H__

#include "Memory/LazyBuffer.h"
#include "Common/Singleton.h"
#include "HttpRequestPref.h"
#include "HttpConnectionPref.h"

class CHttpPrefManager : public CSingleton<CHttpPrefManager>
{
public:
    CHttpRequestPref& GetRequestPref() { return m_RequestPref; }
    CHttpConnectionPref& GetConnectConfig() { return m_ConnectionConfig; }

protected:
    CHttpPrefManager();
    ~CHttpPrefManager();

private:
    CLazyBuffer m_Buffer;
    CHttpRequestPref m_RequestPref;
    CHttpConnectionPref m_ConnectionConfig;

    friend class CSingleton<CHttpPrefManager>;
};

#endif