/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SchemeDefines.h"

#include "URI.h"
#include "URIManager.h"

static CSchemeRegister g_HttpSchemeObject(SCHEME_HTTP,
                                          "http",
                                          URI_ATTR_AUTHORITY_IS_MANDATORY |
                                          URI_ATTR_AUTHORITY_DISALLOW_EMPTY);

static CSchemeRegister g_HttpsSchemeObject(SCHEME_HTTPS,
                                          "https",
                                          URI_ATTR_AUTHORITY_IS_MANDATORY |
                                          URI_ATTR_AUTHORITY_DISALLOW_EMPTY);

static CSchemeRegister g_FileSchemeObject(SCHEME_FILE,
                                          "file",
                                          URI_ATTR_AUTHORITY_IS_MANDATORY |
                                          URI_ATTR_PATH_DISALLOW_EMPTY);

static CSchemeRegister g_FtpSchemeObject(SCHEME_FTP,
                                          "ftp",
                                          URI_ATTR_AUTHORITY_IS_MANDATORY |
                                          URI_ATTR_AUTHORITY_DISALLOW_EMPTY);

static CSchemeRegister g_SipSchemeObject(SCHEME_SIP,
                                          "sip",
                                          URI_ATTR_AUTHORITY_IS_MANDATORY);