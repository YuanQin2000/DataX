/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_CONNECT_SESSION_H__
#define __HTTP_CONNECT_SESSION_H__

#include "Common/Typedefs.h"
#include "Memory/LazyBuffer.h"
#include "DataCom/Controller.h"
#include "HTTPBase/RequestLine.h"

class CUri;
class CHeaderField;
class CStatusLine;
class CConnection;

/**
 * @brief This session controller is used for the HTTP connect request.
 *  HTTP connect request is used for make a HTTP streaming tunnel across the proxy
 *  to the original server, e.g. make a SSL tunnel on the proxy.
 */
class CHttpConnectSession : public CController
{
public:
    CHttpConnectSession(
        CUri* pUri, unsigned short port, bool bSecure);
    ~CHttpConnectSession();

    ErrorCode HandleData(uint8_t* pData, size_t dataLen, size_t* pConsumedLen);
    ErrorCode GenerateData(uint8_t* pBuf, size_t bufLen, size_t* pOutLen);
    bool OnPeerClosed();
    void Reset();

    static CHttpConnectSession* CreateInstance(
        CUri* pUri, unsigned short port, bool bSecure = true);

private:
    ErrorCode HandleConnectResponse(uint8_t* pData, size_t dataLen, size_t* pConsumedLen);

private:
    enum State {
        STATE_INITIATE,
        STATE_SENDING_INVITE,
        STATE_RECVING_RESP,
        STATE_SUCCESS,
        STATE_FAILED
    };

    CLazyBuffer m_Buffer;
    State m_State;
    const bool m_bSecure;
    CRequestLine m_RequestLine;
    CHeaderField* m_pReqHeadField;
    CStatusLine* m_pStatusLine;
    const char* m_pCurPos;
    const char* m_pRequestContentEnd;
    char m_RequestContent[256];

    DISALLOW_DEFAULT_CONSTRUCTOR(CHttpConnectSession);
    DISALLOW_COPY_CONSTRUCTOR(CHttpConnectSession);
    DISALLOW_ASSIGN_OPERATOR(CHttpConnectSession);
};

#endif
