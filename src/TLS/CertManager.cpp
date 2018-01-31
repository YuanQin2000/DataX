/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CertManager.h"
#include "X509Cert.h"
#include "Tracker/Trace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

using std::strerror;

CCertManager::CCertManager() :
    m_pRootCA(".certs/RootCA.crt"),
    m_pMyCert(".certs/MyCert.pem"),
    m_pMyPrivateKey(".certs/MyPrivate.key")
{
    struct stat sb;

    // Check Root CA
    if (stat(m_pRootCA, &sb) == -1) {
        OUTPUT_ERROR_TRACE("stat %s: %s\n", m_pRootCA, strerror(errno));
        ASSERT(false);
        return;
    }
    ASSERT((sb.st_mode & S_IFMT) == S_IFREG, "%s is not a regular file.\n", m_pRootCA);

    // Check Self Certificate
    if (stat(m_pMyCert, &sb) == -1) {
        OUTPUT_ERROR_TRACE("stat %s: %s\n", m_pMyCert, strerror(errno));
        ASSERT(false);
        return;
    }
    ASSERT((sb.st_mode & S_IFMT) == S_IFREG, "%s is not a regular file.\n", m_pMyCert);

    // Check Private Key file
    if (stat(m_pMyPrivateKey, &sb) == -1) {
        OUTPUT_ERROR_TRACE("stat %s: %s\n", m_pMyPrivateKey, strerror(errno));
        ASSERT(false);
        return;
    }
    ASSERT((sb.st_mode & S_IFMT) == S_IFREG, "%s is not a regular file.\n", m_pMyPrivateKey);
}

// TODO: impl
CX509Cert* CCertManager::GetCertByFileName(const char* pCertName)
{
    ASSERT(false);
    return NULL;
}
