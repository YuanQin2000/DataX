/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __URI_SCHEME_DEFINES_H__
#define __URI_SCHEME_DEFINES_H__

/**
 * IANA maintains the registry of URI Schemes [BCP115] at
 *   <http://www.iana.org/assignments/uri-schemes/>.
 */

enum SchemeID {
    SCHEME_UNKNOWN   = -1,
    SCHEME_HTTP,
    SCHEME_HTTPS,
    SCHEME_FILE,
    SCHEME_FTP,
    SCHEME_SMTP,
    SCHEME_SIP,
    SCHEME_COUNT
};

#endif