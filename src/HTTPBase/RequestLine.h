/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTPBASE_REQUEST_LINE_H__
#define __HTTPBASE_REQUEST_LINE_H__

#include "Common/Typedefs.h"
#include "URI/URI.h"
#include "Token.h"

class CRequestLine
{
public:
    CRequestLine(tTokenID method, const char* pTarget, tTokenID version);
    CRequestLine(tTokenID method, tTokenID version);
    CRequestLine(
        tTokenID method,
        CUri* pURI,
        tURISerializeOptions opts,
        unsigned short port,
        tTokenID version);
    ~CRequestLine();

    void SetTarget(CUri* pURI, tURISerializeOptions opts, unsigned short port = 0);
    void SetTarget(const char* pString);
    bool Serialize(char* pBuffer, size_t bufLen, size_t* pOutLen) const;

private:
    enum ValueType {
        VT_INVALID = -1,
        VT_STRING,
        VT_URI
    };

    struct URIValue {
        CUri* pURI;
        tURISerializeOptions Options;
        unsigned short Port;

        URIValue(
            CUri* pValue = NULL,
            tURISerializeOptions opts = URI_SERIALIZE_INVALID,
            unsigned short port = 0) :
            pURI(pValue), Options(opts), Port(port) {}
    };

    union TargetValue {
        URIValue URITarget;
        const char* pStringTarget;

        TargetValue() : URITarget() {}
        TargetValue(const char* pValue) : pStringTarget(pValue) {}
        TargetValue(
            CUri* pValue, tURISerializeOptions opts, unsigned short port) :
            URITarget(pValue, opts, port) {}
        TargetValue(const TargetValue& rhs)
        {
            memcpy(this, &rhs, sizeof(TargetValue));
        }
    };

    tTokenID m_MethodID;
    tTokenID m_VersionID;
    ValueType m_TargetType;
    TargetValue m_Target;
};

#endif
