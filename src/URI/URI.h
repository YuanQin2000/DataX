/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __URI_H__
#define __URI_H__

#include <cstring>
#include <memory>

#include "Common/Typedefs.h"
#include "SchemeDefines.h"
#include "Network/Address.h"

typedef uint32_t tURIAttributions;

#define URI_ATTR_DEFAULT                0x0000

#define URI_ATTR_AUTHORITY_IS_MANDATORY   0x0001
#define URI_ATTR_AUTHORITY_DISALLOW_EMPTY 0x0002
#define URI_ATTR_PATH_DISALLOW_EMPTY      0x0004
#define URI_ATTR_PATH_USE_ROOTLESS        0x0008


typedef uint32_t tURISerializeOptions;

#define URI_SERIALIZE_SCHEME     0x0001
#define URI_SERIALIZE_HOST       0x0002
#define URI_SERIALIZE_PORT       0x0004
#define URI_SERIALIZE_USER_INFO  0x0008
#define URI_SERIALIZE_PATH       0x0010
#define URI_SERIALIZE_QUERY      0x0020
#define URI_SERIALIZE_FRAGMENT   0x0040
#define URI_SERIALIZE_ALL        0xFFFF
#define URI_SERIALIZE_INVALID    0

#define URI_ORIGIN_FORM (URI_SERIALIZE_PATH | URI_SERIALIZE_QUERY)
#define URI_AUTHORITY_FORM (URI_SERIALIZE_HOST | URI_SERIALIZE_PORT)
#define URI_ABSOLUTE_FORM (URI_SERIALIZE_SCHEME | URI_AUTHORITY_FORM | URI_ORIGIN_FORM)

using std::shared_ptr;

class CLazyBuffer;
class CUriBuilder;

class CAuthority
{
public:
    struct HostNameDesc {
        bool bIsIP;
        const char* pName;

        HostNameDesc(bool bIP = false, const char* pStr = NULL) :
            bIsIP(bIP), pName(pStr) {}
    };

public:
    static bool IsValidUsername(const char* pName);
    static bool IsValidHostname(const char* pName, bool* pOutIsIP);

public:
    ~CAuthority();

    bool Serialize(
        char* pBuffer,
        size_t bufLen,
        tURISerializeOptions options,
        unsigned short defaultPort,
        size_t* pOutLen) const;

    const char* UserName() const          { return m_pUserName; }
    const HostNameDesc& HostName() const  { return m_HostName;  }
    unsigned short GetPort(unsigned short defaultValue) const
    {
        return m_Port == 0 ? defaultValue : m_Port;
    }

    tNetworkAddress* GetIPAddress();

private:
    CAuthority(const char* pBuffer,
               const char* pUserName,
               const char* pHostName,
               bool bIPHost,
               unsigned short port);

private:
    const char* const m_pBuffer;     // Buffer owned to store the strings

    const char* const m_pUserName;
    HostNameDesc m_HostName;
    unsigned short m_Port;
    tNetworkAddress m_HostIP;

    friend class CUriBuilder;

    DISALLOW_COPY_CONSTRUCTOR(CAuthority);
    DISALLOW_ASSIGN_OPERATOR(CAuthority);
    DISALLOW_DEFAULT_CONSTRUCTOR(CAuthority);
};


class CUri
{
public:
    ~CUri();

    bool Serialize(
        char* pBuffer,
        size_t bufLen,
        tURISerializeOptions options,
        unsigned short defaultPort,
        size_t* pOutLen) const;

    shared_ptr<CAuthority> Authority() const { return m_pAuthority; }
    SchemeID    Scheme() const       { return m_Scheme;     }
    const char* Path() const         { return m_pPath;      }
    const char* Fragment() const     { return m_pFragment;  }
    const char* Query() const        { return m_pQuery;     }

    enum PathRelation {
        PathNotMatched,
        PathMatched,
        PathIsParent,
        PathIsChild
    };

    /**
     * @return Return the path relation that pPath compared to pBenchMarkPath
     */
    static PathRelation PathCompare(const char* pPath, const char* pBenchMarkPath);

private:
    CUri(const char* pBuffer,
         SchemeID    scheme,
         shared_ptr<CAuthority> pAuthority,
         const char* pPath,
         const char* pQuery,
         const char* pFragment);

private:
    static bool IsValidScheme(const char* pScheme);
    static bool IsValidPath(SchemeID scheme, const char* pPath, bool bHasScheme);
    static bool IsValidQueryFragment(const char* pQueryOrFragment);

    static char* MergePath(tURIAttributions attr, const char* pBase, const char* pPath);
    static char* RemoveDotSegment(char* pPath);

private:
    const char* const m_pBuffer;

    const SchemeID    m_Scheme;
    const char* const m_pPath;
    const char* const m_pQuery;
    const char* const m_pFragment;
    shared_ptr<CAuthority> m_pAuthority;

    friend class CUriBuilder;

    DISALLOW_COPY_CONSTRUCTOR(CUri);
    DISALLOW_ASSIGN_OPERATOR(CUri);
    DISALLOW_DEFAULT_CONSTRUCTOR(CUri);
};


class CUriBuilder
{
public:
    enum BuildErrorID {
        BEID_NONE,
        BEID_SCHEME_INVALID,
        BEID_SCHEME_NOT_REGISTERED,
        BEID_USER_INVALID,
        BEID_HOST_INVALID,
        BEID_PORT_INVALID,
        BEID_PATH_INVALID,
        BEID_QUERY_INVALID,
        BEID_FRAGMENT_INVALID,
        BEID_MISS_BASE_URI,
        BEID_MISS_AUTHORITY,
        BEID_MISS_PATH,
        BEID_MEMORY_FAILURE,
    };

public:
    CUriBuilder();
    virtual ~CUriBuilder();

// Build methods.
public:
    CUri* CreateUriByString(
        const CUri* pBaseURI,
        const char* pString,
        CLazyBuffer* pBufferObject = NULL);
    CUri* CreateUri(
        const char* pScheme,
        const char* pUsername,
        const char* pHostname,
        unsigned short port,
        const char* pPath,
        const char* pQuery,
        const char* pFragment,
        CLazyBuffer* pBufferObject = NULL);

    CUri* CreateUri(
        SchemeID scheme,
        shared_ptr<CAuthority> pAuthority,
        const char* pPath,
        const char* pQuery,
        const char* pFragment,
        CLazyBuffer* pBufferObject = NULL);

    CUri* CreateUri1(
        const CUri* pBaseURI,
        const char* pUsername,
        const char* pHostname,
        unsigned short port,
        const char* pPath,
        const char* pQuery,
        const char* pFragment,
        CLazyBuffer* pBufferObject = NULL);

    CUri* CreateUri1(
        const CUri* pBaseURI,
        shared_ptr<CAuthority> pAuthority,
        const char* pPath,
        const char* pQuery,
        const char* pFragment,
        CLazyBuffer* pBufferObject = NULL);

    CAuthority* CreateAuthorityByString(
        const char* pString, CLazyBuffer* pBufferObject = NULL);

    CAuthority* CreateAuthority(
        const char* pUsername,
        const char* pHostname,
        unsigned short port,
        CLazyBuffer* pBufferObject = NULL);

public:
    BuildErrorID ErrorID()     const { return m_ErrorID;                 }
    const char*  ErrorPhrase() const { return s_ErrorPhrases[m_ErrorID]; }

private:
    CUri* CreateUriSafety(
        SchemeID scheme,
        shared_ptr<CAuthority> pAuthority,
        const char* pPath,
        const char* pQuery,
        const char* pFragment,
        CLazyBuffer* pBufferObject);
    static inline void ReleaseBuffer(void* pBuffer, bool owned)
    {
        if (owned && pBuffer) {
            free(pBuffer);
        }
    }
    static void EmptyDelete(void* pData)
    {
    }

protected:
    BuildErrorID m_ErrorID;

private:
    static const char* s_ErrorPhrases[];
};

#endif
