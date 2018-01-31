/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "URI.h"
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <memory>
#include <alloca.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "URIManager.h"
#include "Memory/LazyBuffer.h"
#include "Network/DNSClient.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::tolower;

namespace NSUriCharUtils
{

inline bool IsUnreservedChar(char c)
{
    return isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

inline bool IsSubDelim(char c)
{
    return c == '!' || c == '$' || c == '&' || c =='\'' || c == '(' ||
           c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
}

inline bool IsGeneralDelim(char c)
{
    return c == ':' || c == '/' || c == '?' ||
           c == '#' || c == '[' || c == ']' || c == '@';
}

/**
* pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
*/
inline bool IsPChar(char c)
{
    return IsUnreservedChar(c) || IsSubDelim(c) || c == ':' || c == '@';
}

inline bool IsReservedChar(char c)
{
    return IsGeneralDelim(c) || IsSubDelim(c);
}

};


/**
 * userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
 * pct-encoded   = "%" HEXDIG HEXDIG
 */
bool CAuthority::IsValidUsername(const char* pName)
{
    char c;
    for (int i = 0; (c = pName[i]) != '\0'; ++i) {
        if (NSUriCharUtils::IsUnreservedChar(c) ||
            NSUriCharUtils::IsSubDelim(c) ||
            c == ':') {
            continue;
        }
        if (c == '%' &&
            NSCharHelper::IsHexChar(pName[i + 1]) &&
            NSCharHelper::IsHexChar(pName[i + 2])) {
            i += 2;
            continue;
        }
        return false;
    }
    return true;    // empty user is valid.
}

/**
 * host          = IP-literal / IPv4address / reg-name
 * 
 * IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
 * reg-name      = *( unreserved / pct-encoded / sub-delims )
 * 
 * TODO: To support IP-literal
 */
bool CAuthority::IsValidHostname(const char* pName, bool* pOutIsIP /* = NULL */)
{
    char c;
    bool bMaybeIP = true;
    int dotNum = 0;
    int IPAddress[4] = { 0 };

    if (pOutIsIP) {
        *pOutIsIP = false;
    }
    for (int i = 0; (c = pName[i]) != '\0'; ++i) {
        if (NSUriCharUtils::IsUnreservedChar(c)) {
            if (bMaybeIP) {
                if (!isdigit(c) && c != '.') {
                    bMaybeIP = false;
                    continue;
                }
                if (c == '.') {
                    if (dotNum < 3 && IPAddress[dotNum] <= 255) {
                        ++dotNum;
                    } else {
                        bMaybeIP = false;
                    }
                } else {    // digit
                    int digit = IPAddress[dotNum];
                    IPAddress[dotNum] = digit * 10 + c - '0';
                }
            }
            continue;
        }
        if (NSUriCharUtils::IsSubDelim(c)) {
            bMaybeIP = false;
            continue;
        }
        if (c == '%' &&
            NSCharHelper::IsHexChar(pName[i + 1]) &&
            NSCharHelper::IsHexChar(pName[i + 2])) {
            bMaybeIP = false;
            i += 2;
            continue;
        }
        return false;
    }
    if (pName[0] != '\0') {
        if (pOutIsIP) {
            *pOutIsIP = bMaybeIP;
        }
        return true;
    }
    return false;
}

CAuthority::CAuthority(
    const char* pBuffer,
    const char* pUserName,
    const char* pHostName,
    bool bIPHost,
    unsigned short port) :
    m_pBuffer(pBuffer),
    m_pUserName(pUserName),
    m_HostName(bIPHost, pHostName),
    m_Port(port),
    m_HostIP(bIPHost ? NSNetworkAddress::GetIPAddress(pHostName) : 0)
{
}

CAuthority::~CAuthority()
{
    if (m_pBuffer) {
        free(const_cast<char*>(m_pBuffer));
    }
}


bool CAuthority::Serialize(
    char* pBuffer,
    size_t bufLen,
    tURISerializeOptions options,
    unsigned short defaultPort,
    size_t* pOutLen) const
{
    ASSERT(pBuffer);
    ASSERT(bufLen > 0);
    ASSERT(pOutLen);

    char* pCur = pBuffer;
    char* pEnd = pBuffer + bufLen;
    *pOutLen = 0;
    if (m_HostName.pName == NULL || (options & URI_SERIALIZE_HOST) == 0) {
        return true;
    }

    // Serialize possible user information
    if (m_pUserName && (options & URI_SERIALIZE_USER_INFO) != 0) {
        if (!NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_pUserName, pOutLen)) {
            return false;
        }
        pCur += *pOutLen;
        if (pEnd > pCur) {
            *pCur++ = '@';
            ++*pOutLen;
        }
        if (pCur == pEnd) {
            return false;
        }
    }

    // Serialize host name
    size_t copied = 0;
    bool bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_HostName.pName, &copied);
    *pOutLen += copied;
    pCur += copied;
    if (!bFinished || pCur == pEnd) {
        return false;
    }

    unsigned port = GetPort(defaultPort);
    if (port > 0 && (options & URI_SERIALIZE_PORT) != 0) {
        *pCur++ = ':';
        ++*pOutLen;
        if (pCur == pEnd) {
            return false;
        }
        size_t copied = 0;
        bool bFinished = NSCharHelper::GetStringByInt(port, pCur, pEnd - pCur, &copied);
        *pOutLen += copied;
        if (!bFinished) {
            return false;
        }
    }
    return true;
}

tNetworkAddress* CAuthority::GetIPAddress()
{
    if (!m_HostIP.IsValid()) {
        ASSERT(!m_HostName.bIsIP);
        uint32_t ipv4 = CDnsClient::Instance()->QueryIPv4Address(m_HostName.pName);
        m_HostIP.SetValue4(ipv4);
        if (!m_HostIP.IsValid()) {
            OUTPUT_ERROR_TRACE("Can not query the host: %s address\n", m_HostName.pName);
            return NULL;
        }
    }
    return &m_HostIP;
}


///////////////////////////////////////////////////////////////////////////////
//
//  CUri Implementation
//
///////////////////////////////////////////////////////////////////////////////

CUri::CUri(
    const char* pBuffer,
    SchemeID    scheme,
    shared_ptr<CAuthority> pAuthority,
    const char* pPath,
    const char* pQueryString,
    const char* pFragment) :
    m_pBuffer(pBuffer),
    m_Scheme(scheme),
    m_pPath(pPath),
    m_pQuery(pQueryString),
    m_pFragment(pFragment),
    m_pAuthority(pAuthority)
{
}

CUri::~CUri()
{
    if (m_pBuffer) {
        free(const_cast<char*>(m_pBuffer));
    }
}

// scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
bool CUri::IsValidScheme(const char* pString)
{
    ASSERT(pString);

    bool bRes = false;
    if (isalpha(*pString)) {
        bRes = true;
        ++pString;
        for (char ch = *pString; (ch = *pString) != '\0'; ++pString) {
            if (!isalnum(ch) && ch != '+' && ch != '-' && ch != '.') {
                bRes = false;
                break;
            }
        }
    }
    return bRes;
}

/**
 * relative-part = "//" authority path-abempty
 *               / path-absolute
 *               / path-noscheme
 *               / path-empty

 * hier-part     = "//" authority path-abempty
 *               / path-absolute
 *               / path-rootless
 *               / path-empty

 * path-abempty  = *( "/" segment )
 * path-absolute = "/" [ segment-nz *( "/" segment ) ]
 * path-noscheme = segment-nz-nc *( "/" segment )
 * path-rootless = segment-nz *( "/" segment )
 * path-empty    = 0<pchar>
 * 
 * segment       = *pchar
 * segment-nz    = 1*pchar
 * segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
                 ; non-zero-length segment without any colon ":"
 */
bool CUri::IsValidPath(SchemeID scheme, const char* pPath, bool bHasScheme)
{
    ASSERT(pPath);

    bool bAllowColon = bHasScheme;
    char ch = *pPath;
    if (ch =='\0') {
        // empty path.
        tURIAttributions attr = CUriManager::Instance()->Attribution(scheme);
        return !(attr & URI_ATTR_PATH_DISALLOW_EMPTY);
    }

    if (ch == '/') {
        // absolute path
        ch = *++pPath;
        bAllowColon = true;
    }

    bool bNeedSegmentOnly = true;
    while (ch != '\0') {
        if (ch == '/') {
            if (bNeedSegmentOnly) {
                return false;
            }
            bNeedSegmentOnly = true;
            bAllowColon = true;
            ch = *++pPath;
            continue;
        }
        bNeedSegmentOnly = false;
        if (ch == ':' && !bAllowColon) {
            return false;
        }

        if (NSUriCharUtils::IsPChar(ch)) {
            ++pPath;
        } else if (ch == '%' &&
                    NSCharHelper::IsHexChar(*(pPath + 1)) &&
                    NSCharHelper::IsHexChar(*(pPath + 2))) {
            pPath += 2;
        } else {
            return false;
        }
        ch = *pPath;
    }
    return true;
}

// query         = *( pchar / "/" / "?" )
// fragment      = *( pchar / "/" / "?" )
bool CUri::IsValidQueryFragment(const char* pQueryOrFragment)
{
    ASSERT(pQueryOrFragment);

    char ch;
    while ((ch = *pQueryOrFragment) != '\0') {
        if (!NSUriCharUtils::IsPChar(ch) && ch != '/' && ch != '?') {
            if (ch == '%' &&
                NSCharHelper::IsHexChar(*(pQueryOrFragment + 1)) &&
                NSCharHelper::IsHexChar(*(pQueryOrFragment + 2))) {
                pQueryOrFragment += 2;
                continue;
            }
            return false;
        }
        ++pQueryOrFragment;
    }
    return true;
}

char* CUri::RemoveDotSegment(char* pPath)
{
    ASSERT(pPath);

    char ch;
    char* pOutCur = pPath;
    char* pInCur = pPath;
    while ((ch = *pInCur) != '\0') {
        if (ch == '.') {
            char ch1 = pInCur[1];
            if (ch1 == '.') {
                char ch2 = pInCur[2];   /*  ../  */
                if (ch2 == '/') {
                    pInCur += 3;
                    continue;
                } else if (ch == '\0') {    /*  ..  */
                    pInCur += 2;
                    continue;
                }
            } else if (ch1 == '/') {    /*  ./  */
                pInCur += 2;
                continue;
            } else if (ch1 == '\0') {   /*  .  */
                ++pInCur;
                continue;
            }
        } else if (ch == '/') {
            char ch1 = pInCur[1];
            if (ch1 == '.') {
                char ch2 = pInCur[2];
                if (ch2 == '/') {   /*  /./   */
                    *pOutCur++ = '/';
                    pInCur += 3;
                    continue;
                } else if (ch2 == '\0') {   /*  /.  */
                    *pOutCur++ = '/';
                    pInCur += 2;
                    continue;
                } else if (ch2 == '.') {
                    char ch3 = pInCur[3];
                    if (ch3 == '/' || ch3 == '\0') {    /*  /../ or /..  */
                        while (pOutCur >= pPath) {
                            if (*pOutCur == '/') {
                                break;
                            }
                            --pOutCur;
                        }
                        *pOutCur = '/';
                        pInCur += 3;
                        if (ch3 == '/') {
                            ++pInCur;
                        }
                        continue;
                    }
                }
            }
        }
        // Copy the segment. (must copy the ch first)
        do {
            *pOutCur++ = ch;
            ch = *++pInCur;
        } while (ch != '/' && ch != '\0');
    }
    *pOutCur = '\0';
    return pPath;
}

bool CUri::Serialize(
    char* pBuffer,
    size_t bufLen,
    tURISerializeOptions options,
    unsigned short defaultPort,
    size_t* pOutLen) const
{
    ASSERT(pBuffer);
    ASSERT(bufLen > 0);
    ASSERT(pOutLen);

    char* pCur = pBuffer;
    char* pEnd = pBuffer + bufLen;
    tURIAttributions attr = CUriManager::Instance()->Attribution(m_Scheme);
    bool bHasAuthority = m_pAuthority.get() ||
                         (attr & URI_ATTR_AUTHORITY_IS_MANDATORY);

    *pOutLen = 0;
    if (options & URI_SERIALIZE_SCHEME) {
        if (!NSCharHelper::CopyNChars(
                pCur, pEnd - pCur,
                CUriManager::Instance()->SchemeID2String(m_Scheme), pOutLen)) {
            return false;
        }
        pCur += *pOutLen;
        if (pCur < pEnd) {
            *pCur++ = ':';
            ++*pOutLen;
        }
        if (pCur == pEnd) {
            return false;
        }
    }
    if (bHasAuthority && (options & URI_SERIALIZE_HOST)) {
        if (options & URI_SERIALIZE_SCHEME) {
            // scheme://authority
            *pCur++ = '/';
            ++*pOutLen;
            if (pCur == pEnd) {
                return false;
            }
            *pCur++ = '/';
            ++*pOutLen;
            if (pCur == pEnd) {
                return false;
            }
        }
        if (m_pAuthority.get()) {
            // Authority Serialize
            size_t copied = 0;
            bool bFinished =
                m_pAuthority->Serialize(pCur, pEnd - pCur, options, defaultPort, &copied);
            *pOutLen += copied;
            pCur += copied;
            if (!bFinished || pCur == pEnd) {
                return false;
            }
        }
    }
    if (options & URI_SERIALIZE_PATH) {
        if (bHasAuthority &&
            !(options & URI_SERIALIZE_HOST) &&
            ((m_pPath && *m_pPath != '/') || !m_pPath)) {
            // Add a '/' in front of the path as this is absolute path.
            *pCur++ = '/';
            ++*pOutLen;
            if (pCur == pEnd) {
                return false;
            }
        }
        if (m_pPath) {
            size_t copied = 0;
            bool bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_pPath, &copied);
            *pOutLen += copied;
            pCur += copied;
            if (!bFinished || pCur == pEnd) {
                return false;
            }
        }
    }

    if (m_pQuery && (options & URI_SERIALIZE_QUERY)) {
        *pCur++ = '?';
        ++*pOutLen;
        if (pCur == pEnd) {
            return false;
        }

        size_t copied = 0;
        bool bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_pQuery, &copied);
        *pOutLen += copied;
        pCur += copied;
        if (!bFinished || pCur == pEnd) {
            return false;
        }
    }
    if (m_pFragment && (options & URI_SERIALIZE_FRAGMENT)) {
        *pCur++ = '#';
        ++*pOutLen;
        if (pCur == pEnd) {
            return false;
        }

        size_t copied = 0;
        bool bFinished = NSCharHelper::CopyNChars(pCur, pEnd - pCur, m_pFragment, &copied);
        *pOutLen += copied;
        pCur += copied;
        if (!bFinished || pCur == pEnd) {
            return false;
        }
    }
    return true;
}

CUri::PathRelation CUri::PathCompare(const char* pPath, const char* pBenchMarkPath)
{
    if (pPath == NULL) {
        pPath = "/";
    }

    const char* pCur1 = pPath;
    const char* pCur2 = pBenchMarkPath;
    char ch1 = *pCur1;
    char ch2 = *pCur2;
    while (ch1 == ch2) {
        if (ch1 == '\0') {
            return PathMatched;    // identical path.
        }
        ch1 = *++pCur1;
        ch2 = *++pCur2;
    }

    PathRelation relation = PathNotMatched;
    if (ch1 == '\0') {
        // pPath is the prefix of string of pBenchMarkPath
        // So pPath may be the parent of pBenchMarkPath
        if (ch2 == '/') {
            relation = *(pCur2 + 1) != '\0' ? PathIsParent : PathMatched;
        } else if (*(pCur2 - 1) == '/') {
            relation = PathIsParent;
        }
    } else if (ch2 == '\0') {
        // pBenchMarkPath is the prefix of string of pPath.
        // So pPath may be a sub path of pBenchMarkPath
        if (ch1 == '/') {
            relation = *(pCur1 + 1) != '\0' ? PathIsChild : PathMatched;
        } else if (*(pCur1 - 1) == '/') {
            relation = PathIsChild;
        }
    }
    return relation;
}

const char* CUriBuilder::s_ErrorPhrases[] = {
    "NONE",
    "Scheme is invalid",
    "Scheme is not registered",
    "User is invalid",
    "Host is invalid",
    "Port is invalid",
    "Path is invalid",
    "Query is invalid",
    "Fragment is invalid",
    "Base URI missed for relative reference",
    "Authority missed (Scheme registered as authority must be presented)",
    "Path content missed (Scheme registered as path must be not empty)",
    "Memory allocate failed",
};

CUriBuilder::CUriBuilder() : m_ErrorID(BEID_NONE)
{
}

CUriBuilder::~CUriBuilder()
{
}

/**
 * URI-reference = URI / relative-ref
 * 
 * URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 * relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
 */
CUri* CUriBuilder::CreateUriByString(
    const CUri* pBaseURI,
    const char* pString,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pString);

    char* pCur = NULL;
    char* pScheme = NULL;
    char* pPath = NULL;
    char* pQuery = NULL;
    char* pFragment = NULL;
    char* pAuthorityString = NULL;
    CAuthority* pAuthorityObj = NULL;
    CUri* pURI = NULL;
    SchemeID scheme = SCHEME_UNKNOWN;
    bool bRelativeRef = false;

    char* pEnd = NULL;
    size_t length = strlen(pString) + 1;
    bool bOwnedBuffer = true;
    char* pBuffer = NULL;
    if (pBufferObject) {
        pBuffer = reinterpret_cast<char*>(pBufferObject->Malloc(length));
        bOwnedBuffer = false;
    } else {
        pBuffer = reinterpret_cast<char*>(malloc(length));
        bOwnedBuffer = true;
    }

    if (!pBuffer) {
        m_ErrorID = BEID_MEMORY_FAILURE;
        return NULL;
    }
    pEnd = pBuffer + length;
    memcpy(pBuffer, pString, length);
    pCur = pBuffer;

    char* pColonLabel = strchr(pBuffer, ':');
    char* pSlashLabel0 = strchr(pBuffer, '/');
    if (pColonLabel && pSlashLabel0 && pColonLabel < pSlashLabel0) {
        *pColonLabel = '\0';
        pScheme = pBuffer;
        if (!CUri::IsValidScheme(pScheme)) {
            m_ErrorID = BEID_SCHEME_INVALID;
            ReleaseBuffer(pBuffer, bOwnedBuffer);
            return NULL;
        }
        if (!CUriManager::Instance()->GetSchemeID(pScheme, scheme)) {
            m_ErrorID = BEID_SCHEME_NOT_REGISTERED;
            ReleaseBuffer(pBuffer, bOwnedBuffer);
            return NULL;
        }
        pCur = pColonLabel + 1;
    } else {    // No scheme, consider it as relative-ref
        bRelativeRef = true;
        if (!pBaseURI) {
            // Don't know relate to which Base URI
            m_ErrorID = BEID_MISS_BASE_URI;
            ReleaseBuffer(pBuffer, pBufferObject == NULL);
            return NULL;
        }
        scheme = pBaseURI->Scheme();
    }

    if (pCur < pEnd && *pCur == '/' && pCur + 1 < pEnd && *(pCur + 1) == '/') {
        pCur += 2;
        pAuthorityString = pCur;
    }

    char* pQuestionMark = strchr(pCur, '?');
    char* pPoundLabel = strchr(pCur, '#');
    if (pQuestionMark) {
        if (!pPoundLabel || (pPoundLabel && pQuestionMark < pPoundLabel)) {
            pQuery = pQuestionMark + 1;
            *pQuestionMark = '\0';
        }
    }
    if (pPoundLabel) {
        pFragment = pPoundLabel + 1;
        *pPoundLabel = '\0';
    }
    char* pSlashLabel = strchr(pCur, '/');
    if (pSlashLabel) {
        if (pAuthorityString && pAuthorityString < pSlashLabel) {
            size_t authorityLen = pSlashLabel - pAuthorityString;
            char* pTmp = reinterpret_cast<char*>(alloca(authorityLen + 1));
            if (!pTmp) {
                m_ErrorID = BEID_MEMORY_FAILURE;
                ReleaseBuffer(pBuffer, bOwnedBuffer);
                return NULL;
            }
            memcpy(pTmp, pAuthorityString, authorityLen);
            pTmp[authorityLen] = '\0';
            pAuthorityString = pTmp;
        }
    }
    if (pAuthorityString) {
        pPath = pSlashLabel;
        if (*pAuthorityString == '\0') {
            pAuthorityString = NULL;
        }
    }
    if (pAuthorityString) {
        pAuthorityObj = CreateAuthorityByString(pAuthorityString, pBufferObject);
        if (!pAuthorityObj) {
            ReleaseBuffer(pBuffer, bOwnedBuffer);
            return NULL;
        }
    } else {
        if (!bRelativeRef &&
            (CUriManager::Instance()->Attribution(scheme) & URI_ATTR_AUTHORITY_DISALLOW_EMPTY)) {
            m_ErrorID = BEID_MISS_AUTHORITY;
            ReleaseBuffer(pBuffer, bOwnedBuffer);
            return NULL;
        }
        pPath = pCur;
    }

    shared_ptr<CAuthority> pAuthority = pBufferObject ?
        shared_ptr<CAuthority>(pAuthorityObj, EmptyDelete) :
        shared_ptr<CAuthority>(pAuthorityObj);

    pURI = bRelativeRef ?
                CreateUri1(pBaseURI, pAuthority, pPath, pQuery, pFragment, pBufferObject) :
                CreateUri(scheme, pAuthority, pPath, pQuery, pFragment, pBufferObject);

    if (!pURI) {
        ReleaseBuffer(pBuffer, bOwnedBuffer);
    }
    return pURI;
}

CUri* CUriBuilder::CreateUri(
    const char* pScheme,
    const char* pUserName,
    const char* pHostName,
    unsigned short port,
    const char* pPath,
    const char* pQuery,
    const char* pFragment,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pScheme);

    CAuthority* pAuthorityObj = NULL;
    SchemeID scheme;

    if (!CUri::IsValidScheme(pScheme)) {
        m_ErrorID = BEID_SCHEME_INVALID;
        return NULL;
    }
    if (!CUriManager::Instance()->GetSchemeID(pScheme, scheme)) {
        // Not registered scheme
        m_ErrorID = BEID_SCHEME_NOT_REGISTERED;
        return NULL;
    }

    if (pHostName) {
        pAuthorityObj = CreateAuthority(pUserName, pHostName, port, pBufferObject);
        if (!pAuthorityObj) {
            return NULL;
        }
    }
    return CreateUri(scheme,
                     shared_ptr<CAuthority>(pAuthorityObj),
                     pPath, pQuery, pFragment,
                     pBufferObject);
}

CUri* CUriBuilder::CreateUri(
    SchemeID scheme,
    shared_ptr<CAuthority> pAuthority,
    const char* pPath,
    const char* pQuery,
    const char* pFragment,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(scheme >= 0 && scheme < SCHEME_COUNT);

    if (!CUriManager::Instance()->IsRegistered(scheme)) {
        m_ErrorID = BEID_SCHEME_NOT_REGISTERED;
        return NULL;
    }

    tURIAttributions attr = CUriManager::Instance()->Attribution(scheme);
    if (!pAuthority.get() &&
        (attr & URI_ATTR_AUTHORITY_DISALLOW_EMPTY)) {
        m_ErrorID = BEID_MISS_AUTHORITY;
        return NULL;
    }

    if (pPath) {
        if (!CUri::IsValidPath(scheme, pPath, true)) {
            m_ErrorID = BEID_PATH_INVALID;
            return NULL;
        }
        size_t len = strlen(pPath) + 1;
        char* pTemp = reinterpret_cast<char*>(alloca(len));
        if (!pTemp) {
            m_ErrorID = BEID_MEMORY_FAILURE;
            return NULL;
        }
        memcpy(pTemp, pPath, len);
        pPath = CUri::RemoveDotSegment(pTemp);
        if (*pPath == '\0') {
            pPath = NULL;
        }
    }
    if (!pPath && (attr & URI_ATTR_PATH_DISALLOW_EMPTY)) {
        m_ErrorID = BEID_MISS_PATH;
        return NULL;
    }

    if (pQuery) {
        if (!CUri::IsValidQueryFragment(pQuery)) {
            m_ErrorID = BEID_QUERY_INVALID;
            return NULL;
        }
        if (*pQuery == '\0') {
            pQuery = NULL;
        }
    }

    if (pFragment) {
        if (!CUri::IsValidQueryFragment(pFragment)) {
            m_ErrorID = BEID_FRAGMENT_INVALID;
            return NULL;
        }
        if (*pFragment == '\0') {
            pFragment = NULL;
        }
    }
    return CreateUriSafety(
        scheme,
        pAuthority,
        pPath, pQuery, pFragment,
        pBufferObject);
}

// Create URI from the Base URI and relative reference
CUri* CUriBuilder::CreateUri1(
    const CUri* pBaseURI,
    const char* pUserName,
    const char* pHostName,
    unsigned short port,
    const char* pPath,
    const char* pQuery,
    const char* pFragment,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pBaseURI);

    shared_ptr<CAuthority> pAuthority;

    if (pHostName) {
        CAuthority* pObj = CreateAuthority(
            pUserName, pHostName, port, pBufferObject);
        if (!pObj) {
            return NULL;
        }
        pAuthority = pBufferObject ?
                     shared_ptr<CAuthority>(pObj, EmptyDelete) :
                     shared_ptr<CAuthority>(pObj);
    }
    return CreateUri1(pBaseURI, pAuthority, pPath, pQuery, pFragment);
}

CUri* CUriBuilder::CreateUri1(
    const CUri* pBaseURI,
    shared_ptr<CAuthority> pAuthority,
    const char* pPath,
    const char* pQuery,
    const char* pFragment,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pBaseURI);

    const char* pBasePath = NULL;
    SchemeID scheme = pBaseURI->Scheme();

    if (pAuthority.get()) {
        return CreateUri(scheme, pAuthority, pPath, pQuery, pFragment, pBufferObject);
    }

    if (pPath && *pPath == '\0') {
        pPath = NULL;
    }
    if (pQuery && *pQuery == '\0') {
        pQuery = NULL;
    }
    if (pQuery && !CUri::IsValidQueryFragment(pQuery)) {
        m_ErrorID = BEID_QUERY_INVALID;
        return NULL;
    }

    pAuthority = pBaseURI->Authority();
    pBasePath = pBaseURI->Path();

    if (!pPath) {
        pPath = pBasePath;
        if (pQuery) {
            if (!CUri::IsValidQueryFragment(pQuery)) {
                m_ErrorID = BEID_QUERY_INVALID;
                return NULL;
            }
        } else {
            pQuery = pBaseURI->Query();
        }
    } else {
        char* pTargetPath = NULL;
        size_t pathLen = 0;

        if (!CUri::IsValidPath(scheme, pPath, false)) {
            m_ErrorID = BEID_PATH_INVALID;
            return NULL;
        }
        pathLen = strlen(pPath);
        if (*pPath == '/') {
            // absolute path, use it directly.
            pTargetPath = reinterpret_cast<char*>(alloca(pathLen + 1));
            if (!pTargetPath) {
                m_ErrorID = BEID_MEMORY_FAILURE;
                return NULL;
            }
            memcpy(pTargetPath, pPath, pathLen);
        } else {
            // non-absolute path, need merge path.
            size_t basePathLen = 0;
            if (!pBasePath) {
                basePathLen = 1;
                pBasePath = "/";
            } else {
                const char* pBasePathEnd = strrchr(pBasePath, '/');
                if (pBasePathEnd) {
                    basePathLen = pBasePathEnd - pBasePath + 1;
                }
            }

            pTargetPath = reinterpret_cast<char*>(alloca(basePathLen + pathLen + 1));
            if (!pTargetPath) {
                m_ErrorID = BEID_MEMORY_FAILURE;
                return NULL;
            }
            if (basePathLen > 0) {
                memcpy(pTargetPath, pBasePath, basePathLen);
            }
            memcpy(pTargetPath + basePathLen, pPath, pathLen + 1);
        }
        CUri::RemoveDotSegment(pTargetPath);
    }
    return CreateUriSafety(scheme,
                           pAuthority,
                           pPath, pQuery, pFragment,
                           pBufferObject);
}

CUri* CUriBuilder::CreateUriSafety(
    SchemeID scheme,
    shared_ptr<CAuthority> pAuthority,
    const char* pPath,
    const char* pQuery,
    const char* pFragment,
    CLazyBuffer* pBufferObject)
{
    size_t pathSize = pPath ? strlen(pPath) + 1 : 0;
    size_t querySize = pQuery ? strlen(pQuery) + 1 : 0;
    size_t fragmentSize = pFragment ? strlen(pFragment) + 1 : 0;

    bool bOwnedBuffer = true;
    char* pBuffer = NULL;
    if (pBufferObject) {
        bOwnedBuffer = false;
        pBuffer = reinterpret_cast<char*>(
            pBufferObject->Malloc(pathSize + querySize + fragmentSize));
    } else {
        pBuffer = reinterpret_cast<char*>(malloc(pathSize + querySize + fragmentSize));
    }
    if (!pBuffer) {
        m_ErrorID = BEID_MEMORY_FAILURE;
        return NULL;
    }
    char* pCur = pBuffer;

    const char* pNewPath = NULL;
    const char* pNewQuery = NULL;
    const char* pNewFragment = NULL;

    if (pPath) {
        pNewPath = pCur;
        memcpy(pCur, pPath, pathSize);
        pCur += pathSize;
    }
    if (pQuery) {
        pNewQuery = pCur;
        memcpy(pCur, pQuery, querySize);
        pCur += querySize;
    }
    if (pFragment) {
        pNewFragment = pCur;
        memcpy(pCur, pFragment, fragmentSize);
        pCur += fragmentSize;
    }
    
    CUri* pURI = NULL;
    if (pBufferObject) {
        void* pMem = pBufferObject->Malloc(sizeof(CUri));
        if (pMem) {
            pURI = new (pMem) CUri(
                NULL, scheme, pAuthority, pNewPath, pNewQuery, pNewFragment);
        }
    } else {
        pURI = new CUri(
            pBuffer, scheme, pAuthority, pNewPath, pNewQuery, pNewFragment);
    }

    if (!pURI) {
        m_ErrorID = BEID_MEMORY_FAILURE;
        ReleaseBuffer(pBuffer, bOwnedBuffer);
    }
    return pURI;
}

CAuthority* CUriBuilder::CreateAuthorityByString(
    const char* pString, CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pString);

    char* pUser = NULL;
    char* pHost = NULL;
    bool bIPHost = false;
    int port = 0;
    size_t length = strlen(pString) + 1;
    char* pBuffer = pBufferObject ?
        reinterpret_cast<char*>(pBufferObject->Malloc(length)) :
        reinterpret_cast<char*>(malloc(length));
    if (!pBuffer) {
        m_ErrorID = BEID_MEMORY_FAILURE;
        return NULL;
    }
    memcpy(pBuffer, pString, length);
    char* pEnd = pBuffer + length;

    /**
     * authority     = [ userinfo "@" ] host [ ":" port ]
     * userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
     * host          = IP-literal / IPv4address / reg-name
     * port          = *DIGIT
     */
    pHost = pBuffer;    // Host is mandatory as the default.
    char* pAtLabel = strchr(pBuffer, '@');
    if (pAtLabel) {
        *pAtLabel = '\0';
        pUser = pBuffer;
        pHost = pAtLabel + 1;
        if (!CAuthority::IsValidUsername(pUser)) {
            m_ErrorID = BEID_USER_INVALID;
            ReleaseBuffer(pBuffer, pBufferObject == NULL);
            return NULL;
        }
        if (*pUser == '\0') {
            pUser = NULL;   // Empty user.
        }
    }

    // We don't support IPv6 address.
    char* pColonLabel = strchr(pHost, ':');
    if (pColonLabel) {
        *pColonLabel = '\0';
        char* pPortString = pColonLabel + 1;
        if (*pPortString != '\0') {
            bool bIsDigitalString =
                NSCharHelper::GetIntByString(pPortString, pEnd - pPortString - 1, &port);
            if (!bIsDigitalString || port < 0 || port > static_cast<unsigned short>(-1)) {
                m_ErrorID = BEID_PORT_INVALID;
                ReleaseBuffer(pBuffer, pBufferObject == NULL);
                return NULL;
            }
        }
    }

    if (!CAuthority::IsValidHostname(pHost, &bIPHost)) {
        m_ErrorID = BEID_HOST_INVALID;
        ReleaseBuffer(pBuffer, pBufferObject == NULL);
        return NULL;
    }

    CAuthority* pAuthority = NULL;
    if (pBufferObject) {
        void* pMem = pBufferObject->Malloc(sizeof(CAuthority));
        if (pMem) {
            pAuthority = new (pMem) CAuthority(
                NULL, pUser, pHost, bIPHost, static_cast<unsigned short>(port));
        }
    } else {
        pAuthority = new CAuthority(
            pBuffer, pUser, pHost, bIPHost, static_cast<unsigned short>(port));
    }
    if (!pAuthority) {
        m_ErrorID = BEID_MEMORY_FAILURE;
        ReleaseBuffer(pBuffer, pBufferObject == NULL);
    }
    return pAuthority;
}

CAuthority* CUriBuilder::CreateAuthority(
    const char* pUser,
    const char* pHost,
    unsigned short port,
    CLazyBuffer* pBufferObject /* = NULL */)
{
    ASSERT(pHost);

    size_t userSize = 0;
    size_t hostSize = 0;
    char* pBuffer = NULL;
    bool bIPHost = false;
    CAuthority* pAuthority = NULL;

    if (!CAuthority::IsValidHostname(pHost, &bIPHost)) {
        m_ErrorID = BEID_HOST_INVALID;
        return NULL;
    }

    if (pUser) {
        if (!CAuthority::IsValidUsername(pUser)) {
            m_ErrorID = BEID_USER_INVALID;
            return NULL;
        }
        if (*pUser != '\0') {
            userSize = strlen(pUser) + 1;
        } else {
            pUser = NULL;
        }
    }
    hostSize = strlen(pHost) + 1;
    pBuffer = pBufferObject ?
        reinterpret_cast<char*>(pBufferObject->Malloc(userSize + hostSize)) :
        reinterpret_cast<char*>(malloc(userSize + hostSize));
    if (pBuffer) {
        char* pCur = pBuffer;
        if (pUser) {
            memcpy(pCur, pUser, userSize);
            pCur += userSize;
        }
        memcpy(pCur, pHost, hostSize);
        if (pBufferObject) {
            void* pMem = pBufferObject->Malloc(sizeof(CAuthority));
            if (pMem) {
                pAuthority = new (pMem) CAuthority(
                    NULL, pUser, pHost, bIPHost, port);
            }
        } else {
            pAuthority = new CAuthority(pBuffer, pUser, pHost, bIPHost, port);
        }
        if (!pAuthority) {
            m_ErrorID = BEID_MEMORY_FAILURE;
            ReleaseBuffer(pBuffer, pBufferObject == NULL);
        }
    } else {
        m_ErrorID = BEID_MEMORY_FAILURE;
    }

    return pAuthority;
}
