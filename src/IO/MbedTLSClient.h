/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __MBED_TLS_CLIENT_H__
#define __MBED_TLS_CLIENT_H__

#ifdef __USE_MBED_TLS__

#include "SSLClient.h"
#include "Common/Typedefs.h"
#include "TLS/X509Cert.h"

#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"

/**
 * @brief SSL client implementation based on the mbed TLS library
 * @see about the mbed OS.
 */
class CMbedTLSClient : public CSSLClient
{
public:
    ~CMbedTLSClient();

    // From CIOContext
    void Close();

private:
    CMbedTLSClient(CIOContext* pIO, bool bCheckPeerCert);    

    // From CSSLClient
    bool Initialize();
    void SetIO(tIOHandle io);
    int Handshake(IOStatus* pStatus);

    // From CIOContext
    size_t DoRead(void* pBuf, size_t len);
    size_t DoWrite(void* pBuf, size_t len);

    static IOStatus HandleError(int sslCode);
    static int VerifyCertificate(
        void* pData, mbedtls_x509_crt* pCrt, int depth, uint32_t* pFlags);

private:
    bool m_bCheckPeerCert;

    uint32_t m_Flags;
    mbedtls_entropy_context m_Entropy;
    mbedtls_ctr_drbg_context m_CtrDrbg;
    mbedtls_ssl_context m_SSL;
    mbedtls_ssl_config m_Config;
    mbedtls_x509_crt m_RootCA;
    mbedtls_x509_crt m_MyCert;
    mbedtls_pk_context m_Privatekey;
    mbedtls_net_context m_NetContext;

    static const char* s_pCustomerString;
    static const size_t s_CustStrLen;

    friend class CSSLClient;

    DISALLOW_COPY_CONSTRUCTOR(CMbedTLSClient);
    DISALLOW_ASSIGN_OPERATOR(CMbedTLSClient);
    DISALLOW_DEFAULT_CONSTRUCTOR(CMbedTLSClient);
};

#endif

#endif