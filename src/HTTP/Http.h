/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_H__
#define __HTTP_H__

#include "Common/Typedefs.h"
#include "HttpRequest.h"
#include "HttpTokenDefs.h"
#include "URI/URI.h"

class CHttp
{
public:
    static CHttpRequest* CreateGetRequest(
        CHttpRequest::IClient& client,
        char* pURIString,
        shared_ptr<CUri> pBaseURI = NULL)
    {
        return CreateRequest(client, NULL, REQUEST_METHOD_GET, pURIString, pBaseURI);
    }

    static CHttpRequest* CreatePostRequest(
        CHttpRequest::IClient& client,
        CSource* pSource,
        char* pURIString,
        shared_ptr<CUri> pBaseURI = NULL)
    {
        return CreateRequest(client, pSource, REQUEST_METHOD_POST, pURIString, pBaseURI);
    }

private:
    static CHttpRequest* CreateRequest(
        CHttpRequest::IClient& client,
        CSource* pSource,
        RequestMethodID method,
        char* pURIString,
        shared_ptr<CUri> pBaseURI);

    DISALLOW_COPY_CONSTRUCTOR(CHttp);
    DISALLOW_ASSIGN_OPERATOR(CHttp);
    DISALLOW_DEFAULT_CONSTRUCTOR(CHttp);
};

#endif