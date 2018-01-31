/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifdef __USE_MBED_TLS__

#include "MbedTLSClient.h"
#include "TLS/CertManager.h"
#include <cstring>

using std::strlen;

const char* CMbedTLSClient::s_pCustomerString = "mbed TLS client";
const size_t CMbedTLSClient::s_CustStrLen = strlen(s_pCustomerString);

CMbedTLSClient::CMbedTLSClient(CIOContext* pIO, bool bCheckPeerCert) :
    CSSLClient(pIO),
    m_bCheckPeerCert(bCheckPeerCert),
    m_Flags(0)
{
    mbedtls_ssl_init(&m_SSL);
    mbedtls_ssl_config_init(&m_Config);
    mbedtls_ctr_drbg_init(&m_CtrDrbg);
    mbedtls_x509_crt_init(&m_RootCA);
    mbedtls_x509_crt_init(&m_MyCert);
    mbedtls_pk_init(&m_Privatekey);
    m_NetContext.fd = INVALID_IO_HANDLE;
}

CMbedTLSClient::~CMbedTLSClient()
{
    Close();
    mbedtls_x509_crt_free(&m_MyCert);
    mbedtls_x509_crt_free(&m_RootCA);
    mbedtls_pk_free(&m_Privatekey);
    mbedtls_ssl_free(&m_SSL);
    mbedtls_ssl_config_free(&m_Config);
    mbedtls_ctr_drbg_free(&m_CtrDrbg);
    mbedtls_entropy_free(&m_Entropy);
}


bool CMbedTLSClient::Initialize()
{
    // 1) Seeding the random number generator
    mbedtls_entropy_init(&m_Entropy);
    int res = mbedtls_ctr_drbg_seed(
        &m_CtrDrbg, mbedtls_entropy_func, &m_Entropy,
        reinterpret_cast<const unsigned char*>(s_pCustomerString), s_CustStrLen);
    if (res != 0) {
        OUTPUT_WARNING_TRACE("mbedtls_ctr_drbg_seed failed: -0x%x\n", -res);
        return false;
    }

    // 2) Loading the CA root certificate
    res = mbedtls_x509_crt_parse_file(&m_RootCA, CCertManager::Instance()->RootCA());
    if(res < 0) {
        OUTPUT_WARNING_TRACE(
            "mbedtls_x509_crt_parse failed -0x%x while parsing root cert\n", -res);
        return false;
    }

    // 3) Loading the client self certifcation.
    res = mbedtls_x509_crt_parse_file(&m_MyCert, CCertManager::Instance()->MyCert());
    if(res < 0) {
        OUTPUT_WARNING_TRACE(
            "mbedtls_x509_crt_parse failed -0x%x while parsing client self cert\n", -res);
        return false;
    }

    // 4) Loading the client private key.
    res = mbedtls_pk_parse_keyfile(&m_Privatekey, CCertManager::Instance()->MyPrivateKey(), "");
    if (res != 0) {
        OUTPUT_WARNING_TRACE(
            "mbedtls_pk_parse_keyfile failed -0x%x while parsing client private key\n", -res);
        return false;
    }

    // 5) Setting up the SSL/TLS configure structure
    res = mbedtls_ssl_config_defaults(
        &m_Config,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM,
        MBEDTLS_SSL_PRESET_DEFAULT);
    if (res != 0) {
        OUTPUT_WARNING_TRACE("mbedtls_ssl_config_defaults failed -0x%x\n", -res);
        return false;
    }

    // 6) Certificate configure options.
//    mbedtls_ssl_conf_verify(&m_Config, VerifyCertificate, NULL);
    mbedtls_ssl_conf_verify(&m_Config, NULL, NULL);
    if (m_bCheckPeerCert) {
        mbedtls_ssl_conf_authmode(&m_Config, MBEDTLS_SSL_VERIFY_REQUIRED);
    } else {
        mbedtls_ssl_conf_authmode(&m_Config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    }
    mbedtls_ssl_conf_rng(&m_Config, mbedtls_ctr_drbg_random, &m_CtrDrbg);

    mbedtls_ssl_conf_ca_chain(&m_Config, &m_RootCA, NULL);
    // Check self certifcate
    res = mbedtls_ssl_conf_own_cert(&m_Config, &m_MyCert, &m_Privatekey);
    if (res != 0) {
        OUTPUT_WARNING_TRACE("mbedtls_ssl_conf_own_cert returned -0x%x\n", -res);
        return false;
    }

    // 7) Link the configuration and the SSL context
    res = mbedtls_ssl_setup(&m_SSL, &m_Config);
    if (res != 0) {
        OUTPUT_WARNING_TRACE("mbedtls_ssl_setup returned -0x%x\n", -res);
        return false;
    }

    return true;
}

void CMbedTLSClient::Close()
{
    int res = 0;
    do {
        res = mbedtls_ssl_close_notify(&m_SSL);
    } while (res == MBEDTLS_ERR_SSL_WANT_WRITE);

    BaseClose();
}

void CMbedTLSClient::SetIO(tIOHandle io)
{
    m_NetContext.fd = io;
    mbedtls_ssl_set_bio(&m_SSL, &m_NetContext, mbedtls_net_send, mbedtls_net_recv, NULL);
}

int CMbedTLSClient::Handshake(IOStatus* pStatus)
{
#ifdef __DEBUG_CERTIFICATE__
    const mbedtls_x509_crt* pPeerCert = NULL;
    char buffer[1024];
#endif

    int res = 0;
    int status = mbedtls_ssl_handshake(&m_SSL);
    switch (status) {
    case 0:
        // complete the hand shake.
        if (m_bCheckPeerCert && mbedtls_ssl_get_verify_result(&m_SSL) != 0) {
            // Verify the peer certificate failed.
            OUTPUT_WARNING_TRACE("Unable to verify peer certificate.\n");
            res = -1;
            *pStatus = IOS_LOCAL_ERROR;
        }
#ifdef __DEBUG_CERTIFICATE__
        if (!m_bCheckPeerCert) {
            pPeerCert = mbedtls_ssl_get_peer_cert(&m_SSL);
            if (pPeerCert) {
                OUTPUT_DEBUG_TRACE("Peer certificate information...\n");
                mbedtls_x509_crt_info(buffer, sizeof(buffer) - 1, "!", pPeerCert);
                OUTPUT_DEBUG_TRACE("%s\n", buf);
            }
        }
#endif
        break;
    case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
        *pStatus = IOS_CLOSED;
        break;
    case MBEDTLS_ERR_SSL_WANT_READ:
    case MBEDTLS_ERR_SSL_WANT_WRITE:
        res = 1;
        break;
    case MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:
        OUTPUT_WARNING_TRACE("Unable to verify peer certificate.\n");
        if (!m_bCheckPeerCert) {
#ifdef __DEBUG_CERTIFICATE__
            pPeerCert = mbedtls_ssl_get_peer_cert(&m_SSL);
            if (pPeerCert) {
                OUTPUT_DEBUG_TRACE("Peer certificate information...\n");
                mbedtls_x509_crt_info(buffer, sizeof(buffer) - 1, "!", pPeerCert);
                OUTPUT_DEBUG_TRACE("%s\n", buf);
            }
#endif
            break;
        }
        // Fall through
    default:
        res = -1;
        *pStatus = IOS_LOCAL_ERROR;
        break;
    }
    return res;
}

size_t CMbedTLSClient::DoRead(void* pBuf, size_t len)
{
    int res = 0;
    if (PrepareIO(&m_Status)) {
        res = mbedtls_ssl_read(&m_SSL, reinterpret_cast<unsigned char*>(pBuf), len);
        if (res < 0) {
            m_Status = HandleError(res);
            res = 0;
        }
    }
    return res;
}

size_t CMbedTLSClient::DoWrite(void* pBuf, size_t len)
{
    int res = 0;
    if (PrepareIO(&m_Status)) {
        res = mbedtls_ssl_write(&m_SSL, reinterpret_cast<unsigned char*>(pBuf), len);
        if (res < 0) {
            m_Status = HandleError(res);
            res = 0;
        }
    }
    return res;
}

CIOContext::IOStatus CMbedTLSClient::HandleError(int sslCode)
{
    CIOContext::IOStatus status = IOS_OK;
    switch (sslCode) {
        case MBEDTLS_ERR_SSL_WANT_READ:
        case MBEDTLS_ERR_SSL_WANT_WRITE:
            status = IOS_NOT_READY;
            break;
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            status = IOS_CLOSED;
            break;
        default:
            OUTPUT_WARNING_TRACE("mbedtls_ssl_write failed, return -0x%x.\n", -sslCode);
            status = IOS_LOCAL_ERROR;
    }
    return status;
}

int CMbedTLSClient::VerifyCertificate(
    void* pData, mbedtls_x509_crt* pCrt, int depth, uint32_t* pFlags)
{
    char buffer[1024];
    OUTPUT_DEBUG_TRACE("Verify requested for (Depth %d):\n", depth);
    OutputTrace("================================================\n");
    mbedtls_x509_crt_info(buffer, sizeof(buffer) - 1, "", pCrt);
    OutputTrace("%s\n", buffer);
    OutputTrace("================================================\n");
    OUTPUT_DEBUG_TRACE("Certificate flag: 0x%x\n", *pFlags);
    return 0;
}

#endif
