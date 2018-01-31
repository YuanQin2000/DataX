/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __NETWORK_DOMAIN_H__
#define __NETWORK_DOMAIN_H__

#include "Common/Typedefs.h"

#define MAX_NUM_OF_DOMAIN_SECTION 12

namespace NSInternetDomain
{
    ///////////////////////////////////////////////////////////////////////////
    //
    // From RFC 6265
    // NOTE: A "public suffix" is a domain that is controlled by a
    //       public registry, such as "com", "co.uk", and "pvt.k12.wy.us".
    //       This step is essential for preventing attacker.com from
    //       disrupting the integrity of example.com by setting a cookie
    //       with a Domain attribute of "com".  Unfortunately, the set of
    //       public suffixes (also known as "registry controlled domains")
    //       changes over time.  If feasible, user agents SHOULD use an
    //       up-to-date public suffix list, such as the one maintained by
    //       the Mozilla project at <http://publicsuffix.org>.
    //
    //
    // Overall, IANA distinguishes the following groups of top-level domains
    //      infrastructure top-level domain (arpa)
    //      country code top-level domains (ccTLD)
    //      internationalized top-level domains (IDNs)
    //          internationalized country code top-level domains, e.g. .xn--fiqs8s
    //          testing top-level domains
    //      generic top-level domains (gTLD)
    //
    // We focus on ccTLD and gTLD.
    //
    ///////////////////////////////////////////////////////////////////////////

    struct DomainDesc {
        const char* pHostDomainName;
        size_t Length;
        bool bWildCard;

        DomainDesc() : pHostDomainName(NULL), Length(0), bWildCard(false) {}
    };

    bool ParseString(const char* pDomainStr, DomainDesc* pOutDesc);
    bool IsMatch(DomainDesc* pDesc, const char* pBenchMarkDomainStr);

    class CDomainSectionAccess
    {
    public:
        CDomainSectionAccess(const char* pDomainStr);
        ~CDomainSectionAccess() {}

        const char* Next()
        {
            if (m_CurrentIndex < m_SectionCount) {
                return m_SectionIndexBuffer[++m_CurrentIndex];
            }
            return NULL;
        }

        const char* Begin()
        {
            return m_SectionIndexBuffer[0];
        }

        const char* PublicSuffix()
        {
            return m_SectionIndexBuffer[m_PublicSuffixIndex];
        }

        const char* RegisteredDomain()
        {
            return m_PublicSuffixIndex > 0 ?
                m_SectionIndexBuffer[m_PublicSuffixIndex - 1] : NULL;
        }

        void Reset()
        {
            m_CurrentIndex = 0;
        }

        // exclude the terminator null character.
        size_t CurrentLength() const
        {
            return m_pDomainStrEnd - m_SectionIndexBuffer[m_CurrentIndex];
        }

    private:
        const char* m_SectionIndexBuffer[MAX_NUM_OF_DOMAIN_SECTION];
        size_t m_PublicSuffixIndex;
        size_t m_CurrentIndex;
        size_t m_SectionCount;
        const char* m_pDomainStrEnd;

        DISALLOW_COPY_CONSTRUCTOR(CDomainSectionAccess);
        DISALLOW_ASSIGN_OPERATOR(CDomainSectionAccess);
        DISALLOW_DEFAULT_CONSTRUCTOR(CDomainSectionAccess);
    };
};

#endif
