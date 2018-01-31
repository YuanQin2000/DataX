/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_SSL_CLIENT_H__
#define __IO_SSL_CLIENT_H__

#include "IOContext.h"
#include "Common/Typedefs.h"
#include "TLS/X509Cert.h"

class CSSLClient : public CIOContext
{
public:
    virtual ~CSSLClient();

    virtual bool Initialize() = 0;

    // From CIOContext
    bool Open();

    static CSSLClient* CreateInstance(CIOContext* pIO, bool bCheckPeerCert);

protected:
    CSSLClient(CIOContext* pTcp);

    void BaseClose()
    {
        if (m_State != STATE_CLOSED) {
            m_State = STATE_CLOSED;
            m_pIO->Close();
        }
    }

    bool PrepareIO(IOStatus* pStatus)
    {
        return (m_State == STATE_OPENED) ? true : Handshake(pStatus) == 0;
    }

private:
    /**
     * @brief Set the SSL read/write IO.
     */
    virtual void SetIO(tIOHandle io) = 0;

    /**
     * @brief Process the SSL handshake.
     * @param pStatus Output paramater for the IO status.
     * @return 0 if complete, >0 if ongoing, <0 if failed
     */
    virtual int Handshake(IOStatus* pStatus) = 0;

private:
    enum InternalState {
        STATE_CLOSED,
        STATE_HANDSHAKE,
        STATE_OPENED
    };

    CIOContext* m_pIO;     // Owned
    InternalState m_State;

    DISALLOW_COPY_CONSTRUCTOR(CSSLClient);
    DISALLOW_ASSIGN_OPERATOR(CSSLClient);
    DISALLOW_DEFAULT_CONSTRUCTOR(CSSLClient);
};

#endif