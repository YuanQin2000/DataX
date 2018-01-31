/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "X509Cert.h"
#include "Tracker/Trace.h"

const char* CX509Cert::CertStatusPhrase(CertStatus status)
{
    static const char* s_Phrase[] = {
        "Certificate OK",
        "Certificate Expire",
        "Certificate is Self Signed",
    };

    ASSERT(status >= 0 && status < CERT_STATUS_COUNT);
    return s_Phrase[status];
}