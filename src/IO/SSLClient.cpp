/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SSLClient.h"
#include "IO/OpenSSLClient.h"
#include "IO/MbedTLSClient.h"

CSSLClient::CSSLClient(CIOContext* pIO) :
    CIOContext(pIO->IsBlockMode()),
    m_pIO(pIO),
    m_State(STATE_CLOSED)
{
}

CSSLClient::~CSSLClient()
{
    delete m_pIO;
}

bool CSSLClient::Open()
{
    if (m_State == STATE_OPENED || m_State == STATE_HANDSHAKE) {
        return true;
    }

    if (!m_pIO->Open()) {
        return false;
    }

    SetIO(m_hIO);

    bool bRes = false;
    do {
        int status = Handshake(&m_Status);
        if (status < 0) {
            // Failed to handshake
            break;
        }
        bRes = true;
        if (status == 0) {
            // Complete handshake.
            m_State = STATE_OPENED;
            break;
        }
        m_State = STATE_HANDSHAKE;
    } while (IsBlockMode());
    return bRes;
}

CSSLClient* CSSLClient::CreateInstance(CIOContext* pIO, bool bCheckPeerCert)
{
    CSSLClient* pInstance = NULL;

#if defined(__USE_OPEN_SSL__)
    pInstance = new COpenSSLClient(pIO, bCheckPeerCert);
#elif defined(__USE_MBED_TLS__)
    pInstance = new CMbedTLSClient(pIO, bCheckPeerCert);
#else
#error "Miss SSL Library"
#endif

    if (pInstance) {
        if (!pInstance->Initialize()) {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}
