/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_CONNECTION_PREF_H__
#define __HTTP_CONNECTION_PREF_H__

#include "Config/ConfigObject.h"

class CLazyBuffer;
class CHttpProxyPref;
class CHttpConnectionPref : public IConfigObject
{
public:
    // From IConfigObject
    bool Initialize(tConfigNode* pConfigData);

    CHttpConnectionPref(CLazyBuffer& buffer);
    ~CHttpConnectionPref();

    CHttpProxyPref* GetHttpProxy() { return m_pHttpProxy; }
    size_t SendBufferSize() const { return m_SendBufferSize; }
    size_t RecvBufferSize() const { return m_RecvBufferSize; }

private:
    bool AnalyzeGlobalAttribution(CForwardList& attrList);

private:
    CLazyBuffer& m_Buffer;
    CHttpProxyPref* m_pHttpProxy;
    size_t m_SendBufferSize;
    size_t m_RecvBufferSize;
};

#endif