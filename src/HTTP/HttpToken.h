/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_TOKEN_H__
#define __HTTP_TOKEN_H__

#include "HTTPBase/Token.h"
#include "HTTPBase/TokenDefs.h"
#include "Common/Singleton.h"
#include "Common/StringIndex.h"

class CHttpTokenMap : public ITokenMap,
                      public CSingleton<CHttpTokenMap>
{
public:
    enum CategoryID {
        CATEGORY_MEDIA_TYPE = TOKEN_GLOBAL_DEFINES_COUNT,
        CATEGORY_ENCODING,
        CATEGORY_CHARSET,
        CATEGORY_CONNECTION,
        CATEGORY_MEDIA_TYPE_PARAM_ID,
        CATEGORY_ACCEPT_EXT_ID,
        CATEGORY_ACCEPT_EXT_VALUE,
        CATEGORY_TRANSFER_EXT,
        CATEGORY_TRANSFER_EXT_PARAM_ID,
        CATEGORY_TRANSFER_EXT_PARAM_VALUE,
        CATEGORY_COOKIE_ATTR_ID,
        CATEGORY_COUNT
    };

    // From ITokenMap
    const char* GetTokenString(int categoryID, int tokenID) const;
    int GetTokenID(int categoryID, const char* pString, size_t len) const;
    bool CheckValidity(tTokenCategory categoryID, tTokenID tokenID) const;

protected:
    CHttpTokenMap();
    ~CHttpTokenMap();

private:
    CStringIndex* m_TokenEntry[CATEGORY_COUNT];

private:
    struct StringArrayEntry {
        size_t Count;
        const char** pArray;
    };
    static const StringArrayEntry s_StringArrays[];

    static const char* s_HttpMethod[];
    static const char* s_HttpVersion[];
    static const char* s_MediaTypes[];
    static const char* s_Encodings[];
    static const char* s_Charsets[];
    static const char* s_Connections[];
    static const char* s_MediaTypeParamToken[];
    static const char* s_AcceptExtToken[];
    static const char* s_AcceptExtTokenValue[];
    static const char* s_TransferEncodingExtToken[];
    static const char* s_TransferEncodingExtParamToken[];
    static const char* s_TransferEncodingExtParamTokenValue[];
    static const char* s_CookieAttrName[];

    friend class CSingleton<CHttpTokenMap>;
};


#endif