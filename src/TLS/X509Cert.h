/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TLS_X509_CERT_H__
#define __TLS_X509_CERT_H__

class CX509Cert
{
public:
    enum CertStatus {
        CERT_OK,
        CERT_EXPIRES,
        CERT_SELF_SIGN,
        CERT_STATUS_COUNT
    };

    static const char* CertStatusPhrase(CertStatus status);
};

#endif