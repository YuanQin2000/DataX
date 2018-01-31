/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "RequestLine.h"
#include "Common/CharHelper.h"
#include "TokenDefs.h"

CRequestLine::CRequestLine(
    tTokenID method, const char* pTarget, tTokenID version) :
    m_MethodID(method),
    m_VersionID(version),
    m_TargetType(VT_STRING),
    m_Target(pTarget)
{
    ASSERT(method);
    ASSERT(version);
    ASSERT(pTarget);
}

CRequestLine::CRequestLine(tTokenID method, tTokenID version) :
    m_MethodID(method),
    m_VersionID(version),
    m_TargetType(VT_INVALID),
    m_Target()
{
}

CRequestLine::CRequestLine(
    tTokenID method,
    CUri* pURI,
    tURISerializeOptions opts,
    unsigned short port,
    tTokenID version) :
    m_MethodID(method),
    m_VersionID(version),
    m_TargetType(VT_URI),
    m_Target(pURI, opts, port)
{
    ASSERT(method);
    ASSERT(version);
    ASSERT(pURI);
    ASSERT(opts != URI_SERIALIZE_INVALID);
}

CRequestLine::~CRequestLine()
{
}

void CRequestLine::SetTarget(
    CUri* pURI, tURISerializeOptions opts, unsigned short port /* = 0 */)
{
    ASSERT(m_TargetType == VT_INVALID);
    ASSERT(pURI);
    ASSERT(opts != URI_SERIALIZE_INVALID);

    m_TargetType = VT_URI;
    m_Target.URITarget.pURI = pURI;
    m_Target.URITarget.Options = opts;
    m_Target.URITarget.Port = port;
}

void CRequestLine::SetTarget(const char* pString)
{
    ASSERT(m_TargetType == VT_INVALID);
    ASSERT(pString);

    m_TargetType = VT_STRING;
    m_Target.pStringTarget = pString;
}

bool CRequestLine::Serialize(char* pBuffer, size_t bufLen, size_t* pOutLen) const
{
    ASSERT(m_TargetType != VT_INVALID);
    ASSERT(pOutLen);

    *pOutLen = 0;
    if (bufLen == 0) {
        return false;
    }

    char* pCur = pBuffer;
    char* pEnd = pBuffer + bufLen;
    const char* pMethodStr =
        CTokenManager::Instance()->GetTokenString(TOKEN_PROTOCOL_METHOD, m_MethodID);

    // Serialize method.
    bool bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, pMethodStr, pOutLen);
    pCur += *pOutLen;
    if (!bFinished || pCur == pEnd) {
        return false;
    }

    *pCur++ = ' ';
    ++*pOutLen;
    if (pCur == pEnd) {
        return false;
    }

    // Serialize Target
    size_t copiedLen = 0;
    bFinished = m_TargetType == VT_URI ?
        m_Target.URITarget.pURI->Serialize(
            pCur, pEnd - pCur, m_Target.URITarget.Options, m_Target.URITarget.Port, &copiedLen) :
        NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_Target.pStringTarget, &copiedLen);
    *pOutLen += copiedLen;
    pCur += copiedLen;
    if (!bFinished || pCur == pEnd) {
        return false;
    }
    *pCur++ = ' ';
    ++*pOutLen;
    if (pCur == pEnd) {
        return false;
    }

    // Serialize Version
    const char* pVersionStr =
        CTokenManager::Instance()->GetTokenString(TOKEN_PROTOCOL_VERSION, m_VersionID);
    bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, pVersionStr, &copiedLen);
    *pOutLen += copiedLen;
    pCur += copiedLen;
    if (!bFinished || pCur == pEnd) {
        return false;
    }

    // Terminator \r\n
    *pCur++ = '\r';
    ++*pOutLen;
    if (pCur == pEnd) {
        return false;
    }
    *pCur++ = '\n';
    ++*pOutLen;

    return true;
}
