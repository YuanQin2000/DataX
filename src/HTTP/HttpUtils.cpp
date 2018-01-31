/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpUtils.h"
#include "Common/Limits.h"
#include "URI/URI.h"
#include "HTTPBase/HeaderField.h"
#include "HttpHeaderFieldDefs.h"
#include "HttpPrefManager.h"
#include "HttpCookieManager.h"
#include "HttpFieldValue.h"

namespace NSHttpUtils
{

CHeaderField* CreateDefaultRequestHeaderField(
    CLazyBuffer& buffer, bool bFillPref /* = true */)
{
    CHeaderField::FieldInitedValue* pInitValue = NULL;
    if (bFillPref) {
        CHttpRequestPref& requestPref(CHttpPrefManager::Instance()->GetRequestPref());
        pInitValue = requestPref.GetFieldInitedValue();
    }
    return CHeaderField::CreateInstance(
        CHttpHeaderFieldDefs::GetRequestGlobalConfig(), buffer, pInitValue);
}

bool SetHostField(CAuthority* pHost, CHeaderField* pHeaderField)
{
    bool bRes = false;
    size_t hostLength = 0;
    char buffer[MAX_LENGTH_OF_HOSTNAME];

    if (pHost->Serialize(
        buffer, sizeof(buffer) - 1, URI_SERIALIZE_HOST, 0, &hostLength)) {
        buffer[hostLength] = '\0';
        pHeaderField->SetFieldValue(CHttpHeaderFieldDefs::REQ_FN_HOST, buffer);
        bRes = true;
    }
    return bRes;
}

void SetCookieField(CUri* pUri, CHeaderField* pHeaderField)
{
    CLazyBuffer& sharedBuffer(pHeaderField->GetBuffer());
    CHttpCookieValueList cookies(&sharedBuffer);
    if (!CHttpCookieManager::GetDefaultInstance()->GetCookies(pUri, &cookies)) {
        // Get Cookie failed
        return;
    }
    CForwardList& cookiesList(cookies.GetList());
    if (cookiesList.Count() == 0) {
        // No Cookie
        return;
    }

    CForwardList::Iterator iter = cookiesList.Begin();
    CForwardList::Iterator iterEnd = cookiesList.End();
    void* pMem = sharedBuffer.Malloc(sizeof(CForwardList));
    if (pMem == NULL) {
        OUTPUT_WARNING_TRACE("Create Field Value List Failed.\n");
        return;
    }
    CForwardList* pCookieHFObjectList = new (pMem) CForwardList(&sharedBuffer);
    while (iter != iterEnd) {
        HttpCookieValue* pCookieValue =
            reinterpret_cast<HttpCookieValue*>(cookiesList.DataAt(iter));
        void* pCookieMem = sharedBuffer.Malloc(sizeof(CHttpCookieFieldValue));
        if (pCookieMem) {
            CHttpCookieFieldValue* pCookieFieldValue =
                new (pCookieMem) CHttpCookieFieldValue(pCookieValue);
            pCookieHFObjectList->PushBack(pCookieFieldValue);
        }
        ++iter;
    }
    pHeaderField->SetFieldValue(CHttpHeaderFieldDefs::REQ_FN_COOKIE, pCookieHFObjectList);
}

CompressType Encoding2CompressType(EncodingType type)
{
    switch (type) {
    case ENCODING_NONE:     return CT_NONE;
    case ENCODING_XCOMPRESS:
    case ENCODING_COMPRESS: return CT_COMPRESS;
    case ENCODING_XGZIP:
    case ENCODING_GZIP:     return CT_GZIP;
    case ENCODING_DEFLAT:   return CT_DEFLAT;
    default:
        ASSERT(false);
        break;
    }
    return CT_NONE;
}

EncodingType Compress2EncodingType(CompressType ct)
{
    switch (ct) {
    case CT_NONE:     return ENCODING_NONE;
    case CT_COMPRESS: return ENCODING_COMPRESS;
    case CT_GZIP:     return ENCODING_GZIP;
    case CT_DEFLAT:   return ENCODING_DEFLAT;
    default:
        ASSERT(false);
        break;
    }
    return ENCODING_NONE;
}

};