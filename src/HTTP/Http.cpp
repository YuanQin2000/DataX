/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Http.h"
#include <alloca.h>
#include "HttpTokenDefs.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

CHttpRequest* CHttp::CreateRequest(
    CHttpRequest::IClient& client,
    CSource* pSource,
    RequestMethodID method,
    char* pURIString,
    shared_ptr<CUri> pBaseURI)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    const CUri* pBase = pBaseURI.get();
    char* pFullURIString = pURIString;

    // Canonicalized: upper case -> lower case.
    NSCharHelper::String2LowerCase(pURIString);

    if (!pBase) {
        static const char s_HttpPrefix[] = "http://";
        static const char s_HttpsPrefix[] = "https://";
        if (strncmp(pURIString, s_HttpPrefix, sizeof(s_HttpPrefix) - 1) != 0 &&
            strncmp(pURIString, s_HttpsPrefix, sizeof(s_HttpsPrefix) - 1) != 0) {
            pFullURIString = reinterpret_cast<char*>(alloca(strlen(pURIString) + sizeof(s_HttpPrefix)));
            if (!pFullURIString) {
                OUTPUT_ERROR_TRACE("Alloca memory on stack failed\n");
                return NULL;
            }
            strcpy(pFullURIString, s_HttpPrefix);
            strcpy(pFullURIString + sizeof(s_HttpPrefix) - 1, pURIString);
        }
    }

    CUriBuilder uriBuilder;
    CUri* pURI = uriBuilder.CreateUriByString(pBase, pFullURIString);
    if (!pURI) {
        OUTPUT_ERROR_TRACE("Create URI failed: %s\n", uriBuilder.ErrorPhrase());
        return NULL;
    }
    shared_ptr<CUri> pTarget(pURI);
    return CHttpRequest::CreateInstance(client, pSource, method, pTarget);
}
