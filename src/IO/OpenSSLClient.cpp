/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifdef __USE_OPEN_SSL__

#include "OpenSSLClient.h"
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "Common/ErrorNo.h"
#include "Tracker/Trace.h"

using std::strerror;
using std::atexit;

COpenSSLClient::COpenSSLClient(CIOContext* pIO, bool bCheckPeerCert) :
    CSSLClient(pIO),
    m_pSSL(NULL),
    m_bCheckPeerCert(bCheckPeerCert)
{
}

COpenSSLClient::~COpenSSLClient()
{
    if (m_pSSL) {
        SSL_free(m_pSSL);
    }
}

bool COpenSSLClient::Initialize()
{
    m_pSSL = SSL_new(CSSLContext::GetInstance()->GetData());
    if (m_pSSL) {
        return true;
    }

    OUTPUT_ERROR_TRACE("SSL_new failed\n");
    ERR_print_errors_fp(stderr);
    return false;
}

void COpenSSLClient::Close()
{
    SSL_shutdown(m_pSSL);
    BaseClose();
}

void COpenSSLClient::SetIO(tIOHandle io)
{
    SSL_set_fd(m_pSSL, io);
}

int COpenSSLClient::Handshake(IOStatus* pStatus)
{
    int res = 0;
    int status = SSL_connect(m_pSSL);
    if (status == -1) {
        if (HandleError(SSL_get_error(m_pSSL, res), pStatus)) {
            res = 1;
        } else {
            OUTPUT_WARNING_TRACE("SSL_connect: %s\n", strerror(errno));
            ERR_print_errors_fp(stderr);
            res = -1;
        }
    }
    return res;
}

size_t COpenSSLClient::DoRead(void* pBuf, size_t len)
{
    int res = 0;
    if (PrepareIO(&m_Status)) {
        res = SSL_read(m_pSSL, pBuf, len);
        if (res <= 0) {
            HandleError(SSL_get_error(m_pSSL, res), &m_Status);
            res = 0;
        }
    }
    return res;
}

size_t COpenSSLClient::DoWrite(void* pBuf, size_t len)
{
    int res = 0;
    if (PrepareIO(&m_Status)) {
        res = SSL_write(m_pSSL, pBuf, len);
        if (res <= 0) {
            HandleError(SSL_get_error(m_pSSL, res), &m_Status);
            res = 0;
        }
    }
    return res;
}

bool COpenSSLClient::HandleError(int errorCode, IOStatus* pStatus)
{
    bool bTransientError = false;
    int sysErrorNo = 0;

    switch (errorCode) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
        bTransientError = true;
        *pStatus = IOS_NOT_READY;
        break;
    case SSL_ERROR_ZERO_RETURN:
        //Transport layer has been closed, we will not continued.
        *pStatus = IOS_CLOSED;
        break;
    case SSL_ERROR_SYSCALL:
        sysErrorNo = errno;
        if (sysErrorNo == EINTR) {
            bTransientError = true;
        } else if (sysErrorNo == EAGAIN) {
            bTransientError = true;
            *pStatus = IOS_NOT_READY;
        } else if (sysErrorNo != 0) {
            OUTPUT_NOTICE_TRACE("SSL error: %s\n", strerror(sysErrorNo));
            *pStatus = IOS_LOCAL_ERROR;
            SET_ERROR_CODE(sysErrorNo);
        }
        break;
    case SSL_ERROR_SSL:
    case SSL_ERROR_WANT_X509_LOOKUP:
    case SSL_ERROR_WANT_CONNECT:
    case SSL_ERROR_WANT_ACCEPT:
    default:
        OUTPUT_NOTICE_TRACE("SSL error: %d\n", errorCode);
        *pStatus = IOS_LOCAL_ERROR;
        SET_ERROR_CODE(EIO);
        break;
    }

    return bTransientError;
}

CX509Cert::CertStatus COpenSSLClient::CheckCertificate(SSL* pSSL)
{
    // TODO: Impl
#if 0
    X509* pCert = SSL_get_peer_certificate(pSSL);
    X509_free(pCert);
#endif
    return CX509Cert::CERT_OK;
}


COpenSSLClient::CSSLContext::~CSSLContext()
{
    if (m_pContext) {
        SSL_CTX_free(m_pContext);
        m_pContext = NULL;
    }
}

bool COpenSSLClient::CSSLContext::Initialize()
{
    // The library init function will load the openssl share library on system level, so you have to
    // 1) install openssl on the system OR
    // 2) copy your compiled/customerized libraries to /usr/lib
    // 3) Set the environment variables LD_LIBRARY_PATH to
    //    your customerized libraries path before running this application
    ASSERT(m_pContext == NULL);

    if (!SSL_library_init()) {
        OUTPUT_ERROR_TRACE("SSL_library_init failed!\n");
        return NULL;
    }
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    m_pContext = SSL_CTX_new(SSLv23_client_method());
    return m_pContext != NULL;
}

#endif
