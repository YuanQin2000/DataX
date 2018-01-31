/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_OPEN_SSL_CLIENT_H__
#define __IO_OPEN_SSL_CLIENT_H__

#ifdef __USE_OPEN_SSL__

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "SSLClient.h"
#include "Common/Typedefs.h"
#include "Common/Singleton.h"
#include "TLS/X509Cert.h"

/**
 * @brief SSL Client based on the open ssl library.
 */
class COpenSSLClient : public CSSLClient
{
public:
    ~COpenSSLClient();

    // From CIOContext
    void Close();

private:
    class CSSLContext : public CSingleton2<CSSLContext>
    {
    public:
        ~CSSLContext();
        SSL_CTX* GetData() { return m_pContext; }

    private:
        CSSLContext() : m_pContext(NULL) {}
        bool Initialize();

    private:
        SSL_CTX* m_pContext;

        friend class CSingleton2<CSSLContext>;
    };

    COpenSSLClient(CTcpClient* pTcp, bool bCheckPeerCert);

    // From CSSLClient
    bool Initialize();
    void SetIO(tIOHandle io);
    int Handshake(IOStatus* pStatus);

    // From CIOContext
    size_t DoRead(void* pBuf, size_t len);
    size_t DoWrite(void* pBuf, size_t len);

    /**
     * @brief Handle the open SSL internal errors.
     * @param errorCode The SSL internal error code.
     * @param pStatus The output parameter for the IO status.
     * @return true if the error is transient (recoverable), false otherwise
     */
    static bool HandleError(int errorCode, IOStatus* pStatus);

    /**
     * @brief Check the peer certificate validity.
     * @param pSSL the SSL connected to the peer.
     * @return The X509 certificate status.
     */
    static CX509Cert::CertStatus CheckCertificate(SSL* pSSL);

private:
    SSL* m_pSSL;
    bool m_bCheckPeerCert;

    friend class CSSLClient;

    DISALLOW_COPY_CONSTRUCTOR(COpenSSLClient);
    DISALLOW_ASSIGN_OPERATOR(COpenSSLClient);
    DISALLOW_DEFAULT_CONSTRUCTOR(COpenSSLClient);
};

#endif

#endif