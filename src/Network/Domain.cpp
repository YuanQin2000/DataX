/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Domain.h"
#include <cctype>
#include <set>
#include "Common/Macros.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::isalnum;
using std::memcpy;
using std::memcmp;
using std::set;

static int MakeSectionIndex(
    const char* pDomainStr,
    const char** pIdxBuf, size_t idxBufLen,
    size_t* pOutDomainStrLen = NULL);
static int FindPublicSuffix(const char** pIndex, size_t indexLen);
static bool IsConuntryCode(const char* pStr, size_t len = 0);
static bool IsGeneralTopDomain(const char* pStr, size_t len = 0);

namespace NSInternetDomain
{

///////////////////////////////////////////////////////////////////////////////
//
// A simple implementation by checking the length and content of domain.
//
///////////////////////////////////////////////////////////////////////////////
bool ParseString(const char* pDomainStr, DomainDesc* pOutDesc)
{
    ASSERT(pDomainStr);
    ASSERT(pOutDesc);

    const char* pString = pDomainStr;
    bool bWildCard = false;

    if (*pString == '.') {
        bWildCard = true;
        ++pString;
    }

    size_t strLen = 0;
    const char* indexBuf[MAX_NUM_OF_DOMAIN_SECTION];
    int sectionCount = MakeSectionIndex(
        pString, indexBuf, COUNT_OF_ARRAY(indexBuf), &strLen);
    if (sectionCount <= 0) {
        OUTPUT_ERROR_TRACE("Make domain section index failed: %s\n", pString);
        return false;
    }

    int publicSuffixPos = FindPublicSuffix(indexBuf, sectionCount);
    if (publicSuffixPos > 0) {
        pOutDesc->pHostDomainName = pString;
        pOutDesc->Length = strLen;
        pOutDesc->bWildCard = bWildCard;
        return true;
    }
    OUTPUT_ERROR_TRACE("Domain String is NOT an available registered domain: %s\n", pString);
    return false;
}

bool IsMatch(DomainDesc* pDesc, const char* pBenchMarkDomainStr)
{
    if (pDesc == NULL) {
        return pBenchMarkDomainStr == NULL;
    }

    const char* pDomainStr = pDesc->pHostDomainName;
    size_t len1 = pDesc->Length;
    size_t len2 = strlen(pBenchMarkDomainStr);
    if (len1 > len2 || len1 == 0 || len2 == 0) {
        return false;
    }
    if (len1 == len2) {
        return memcmp(pDomainStr, pBenchMarkDomainStr, len1) == 0;
    }

    bool bMatched = false;
    const char* pCur1 = pDomainStr + len1 - 1;
    const char* pCur2 = pBenchMarkDomainStr + len2 - 1;
    while (*pCur1 == *pCur2) {
        --pCur2;
        if (pCur1 > pDomainStr) {
            --pCur1;
        } else {
            // pCur1 == pDomainStr
            bMatched = true;
            break;
        }
    }
    return (bMatched && *pCur2 == '.');
}


CDomainSectionAccess::CDomainSectionAccess(const char* pDomainStr) :
    m_SectionIndexBuffer{0},
    m_PublicSuffixIndex(0),
    m_CurrentIndex(0),
    m_SectionCount(0),
    m_pDomainStrEnd(NULL)
{
    ASSERT(pDomainStr);

    size_t strLen = 0;
    int sectionCount = MakeSectionIndex(
        pDomainStr, m_SectionIndexBuffer, COUNT_OF_ARRAY(m_SectionIndexBuffer), &strLen);
    if (sectionCount > 0) {
        int publicSuffixIndex = FindPublicSuffix(m_SectionIndexBuffer, sectionCount);
        if (publicSuffixIndex > 0) {
            m_PublicSuffixIndex = publicSuffixIndex;
        }
        m_SectionCount = sectionCount;
        m_pDomainStrEnd = pDomainStr + strLen;
    } else {
        OUTPUT_ERROR_TRACE("MakeSectionIndex failed on %s\n", pDomainStr);
    }
}

};



static int MakeSectionIndex(
    const char* pDomainStr,
    const char** pIdxBuf, size_t idxBufLen,
    size_t* pOutDomainStrLen /* = NULL */)
{
    ASSERT(idxBufLen > 0);

    const char* pCur = pDomainStr;
    size_t i = 0;
    size_t length = 0;
    pIdxBuf[i++] = pCur;
    char ch = *pCur;
    while (ch != '\0') {
        ++length;
        if (ch == '-') {
            if (i == 0 || pCur[1] == '\0') {
                // begin/end with hyphen. invalid domain.
                i = 0;
                break;
            }
        } else if (ch == '.') {
            if (i >= idxBufLen || i == 0 || pCur[1] == '\0' || pCur[1] == '.') {
                i = 0;
                break;
            }
            pIdxBuf[i++] = pCur + 1;
        } else if (!isalnum(ch)) {
            i = 0;
            break;
        }
        ch = *++pCur;
    }
    if (pOutDomainStrLen) {
        *pOutDomainStrLen = length;
    }
    return i;
}

static int FindPublicSuffix(const char** pIndex, size_t indexLen)
{
    ASSERT(indexLen >= 1);

    size_t pos = indexLen - 1;
    const char* pCountryCodeStr = NULL;
    const char* pTopDomainStr = NULL;
    size_t ccLen = 0;
    size_t tdLen = 0;
    size_t len = strlen(pIndex[pos]);

    if (len < 2 || len > 63) {
        return -1;
    }
    if (len == 2) {
        pCountryCodeStr = pIndex[pos];
        ccLen = len;
        if (pos > 0) {
            --pos;
            pTopDomainStr = pIndex[pos];
            tdLen = pCountryCodeStr - pTopDomainStr - 1;
        }
    } else {
        pTopDomainStr = pIndex[pos];
        tdLen = len;
    }

    if (pCountryCodeStr && !IsConuntryCode(pCountryCodeStr, ccLen)) {
        return -1;
    }
    if (pTopDomainStr) {
        if (!IsGeneralTopDomain(pTopDomainStr, tdLen)) {
            if (pos < indexLen - 1) {
                ++pos;
            } else {
                return -1;
            }
        }
    }
    return pos;
}

static bool IsConuntryCode(const char* pStr, size_t len /* = 0 */)
{
    ///////////////////////////////////////////////////////////////////////////
    // Country Code List:
    //
    // https://en.wikipedia.org/wiki/List_of_Internet_top-level_domains#Country_code_top-level_domains
    ///////////////////////////////////////////////////////////////////////////
    static const uint32_t s_CountryCodeMap[26] = {
        //  a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z
        //  0,  0, ac, ad, ae, af, ag,  0, ai,  0,  0, al, am, an, ao,  0, aq, ar, as, at, au,  0, aw, ax,  0, az
        // (0000 00)00 1111 1010 0111 1011 1110 1101,
        0x00FA7BEDU,
        // ba, bb,  0, bd, be, bf, bg, bh, bi, bj,  0, bl, bm, bn, bo,  0, bq, br, bs, bt,  0, bv, bw,  0, by, bz
        // 11011111110111101111011011
        0x037F7BDBU,
        // ca,  0, cc, cd,  0, cf, cg, ch, ci,  0, ck, cl, cm, cn, co,  0,  0, cr,  0,  0, cu, cv, cw, cx, cy, cz
        // 10 1101 1110 1111 1001 0011 1111
        0x02DEF93FU,
        //  0,  0,  0,  0, de,  0,  0,  0,  0, dj, dk,  0, dm,  0, do,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, dz
        // 00001000011010100000000001
        0x0021A801U,
        //  0,  0, ec,  0, ee,  0, eg, eh,  0,  0,  0,  0,  0,  0,  0,  0,  0, er, es, et, eu,  0,  0,  0,  0,  0
        // 00101011000000000111100000
        0x00AC01E0U,
        //  0,  0,  0,  0,  0,  0,  0,  0, fi, fj, fk,  0, fm,  0, fo,  0,  0, fr,  0,  0,  0,  0,  0,  0,  0,  0
        // 00000000111010100100000000
        0x0003A900U,
        // ga, gb,  0, gd, ge, gf, gg, gh, gi,  0,  0, gl, gm, gn,  0, gp, gq, gr, gs, gt, gu,  0, gw,  0, gy,  0
        // 11011111100111011111101010
        0x037E77EAU,
        //  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, hk,  0, hm, hn,  0,  0,  0, hr,  0, ht, hu,  0,  0,  0,  0,  0
        // 00000000001011000101100000
        0x00000B16U,
        //  0,  0,  0, id, ie,  0,  0,  0,  0,  0,  0, il, im, in, io,  0, iq, ir, is, it,  0,  0,  0,  0,  0,  0
        // 00011000000111101111000000
        0x00607BC0U,
        //  0,  0,  0,  0, je,  0,  0,  0,  0,  0,  0,  0, jm,  0, jo, jp,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        // 00001000000010110000000000
        0x00202C00U,
        //  0,  0,  0,  0, ke,  0, kg, kh, ki,  0,  0,  0, km, kn,  0, kp,  0, kr,  0,  0,  0,  0, kw,  0, ky, kz
        // 00001011100011010100001011
        0x002E350BU,
        // la, lb, lc,  0,  0,  0,  0,  0, li,  0, lk,  0,  0,  0,  0,  0,  0, lr, ls, lt, lu, lv,  0,  0, ly,  0
        // 11100000101000000111110010
        0x038281F2U,
        // ma,  0, mc, md, me, mf, mg, mh,  0,  0, mk, ml, mm, mn, mo, mp, mq, mr, ms, mt, mu, mv, mw, mx, my, mz
        // 10111111001111111111111111
        0x02FCFFFFU,
        // na,  0, nc,  0, ne, nf, ng,  0, ni,  0,  0, nl,  0,  0, no, np,  0, nr,  0,  0, nu,  0,  0,  0,  0, nz
        // 10101110100100110100100001
        0x02BA4D21U,
        //  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, om,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        // 00000000000010000000000000
        0x00002000U,
        // pa,  0,  0,  0, pe, pf, pg, ph,  0,  0, pk, pl, pm, pn,  0,  0,  0, pr, ps, pt,  0,  0, pw,  0, py,  0
        // 10001111001111000111001010
        0x023CF1CAU,
        // qa,  0,....0
        0x0,
        //  0,  0,  0,  0, re,  0,  0,  0,  0,  0,  0,  0,  0,  0, ro,  0,  0,  0, rs,  0, ru,  0, rw,  0,  0,  0
        // 00001000000000100010101000
        0x002008A8U,
        // sa, sb, sc, sd, se,  0, sg, sh, si, sj, sk, sl, sm, sn, so,  0,  0, sr, ss, st, su, sv,  0, sx, sy, sz
        // 11111011111111100111110111
        0x03EFF9F7U,
        //  0,  0, tc, td,  0, tf, tg, th,  0, tj, tk, tl, tm, tn, to, tp,  0, tr,  0, tt,  0, tv, tw,  0,  0, tz
        // 00110111011111110101011001
        0x00DDFD59U,
        // ua,  0,  0,  0,  0,  0, ug,  0,  0,  0, uk,  0, um,  0,  0,  0,  0,  0, us,  0,  0,  0,  0,  0, uy, uz
        // 10000010001010000010000011
        0x0208A083U,
        // va,  0, vc,  0, ve,  0, vg,  0, vi,  0,  0,  0,  0, vn,  0,  0,  0,  0,  0,  0, vu,  0,  0,  0,  0,  0
        // 10101010100001000000100000
        0x02AA1020U,
        //  0,  0,  0,  0,  0, wf,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, ws,  0,  0,  0,  0,  0,  0,  0
        // 00000100000000000010000000
        0x00100080U,
        //  0, ..., 0
        0x0,
        //  0,  0,  0,  0, ye,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, yt,  0,  0,  0,  0,  0,  0
        // 00001000000000000001000000
        0x00200040U,
        // za,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, zm,  0,  0,  0,  0,  0,  0,  0,  0,  0, zw,  0,  0,  0
        // 10000000000010000000001000
        0x02002008U
    };

    bool bRes = false;
    size_t strLen = len;
    if (strLen == 0) {
        strLen = strlen(pStr);
    }
    if (strLen == 2) {
        int idx1 = pStr[0] - 'a';
        if (idx1 >= 0 && idx1 < static_cast<int>(COUNT_OF_ARRAY(s_CountryCodeMap))) {
            int idx2 = pStr[1] - 'a';
            if (idx2 >= 0 && idx2 < 32) {
                uint32_t flag = (0x80000000 >> idx2);
                bRes = ((flag & s_CountryCodeMap[idx1]) != 0);
            }
        }
    }
    return bRes;
}

static bool IsGeneralTopDomain(const char* pStr, size_t len /* = 0 */)
{
    ///////////////////////////////////////////////////////////////////////////
    //
    // Reference:
    // https://en.wikipedia.org/wiki/Top-level_domain
    // https://en.wikipedia.org/wiki/Generic_top-level_domain
    //
    ///////////////////////////////////////////////////////////////////////////
    static const char* s_TopDomainArray[] = {
        "com",
        "net",
        "org",
        "gov",
        "edu",
        "mil",  // military
        "biz",
        "info",
        "name",
        "jobs",
        "aero",
    };

    static const set<const char*, tStringCompareFunc> s_TopDomainMap(
        s_TopDomainArray,
        s_TopDomainArray + COUNT_OF_ARRAY(s_TopDomainArray),
        NSCharHelper::StringCaseCompare);

    bool bRes = false;
    size_t strLen = len;

    if (strLen == 0) {
        strLen = strlen(pStr);
    }
    if (strLen > 2 && strLen < 64) {
        set<const char*>::const_iterator iter = s_TopDomainMap.find(pStr);
        bRes = (iter != s_TopDomainMap.end());
    }
    return bRes;
}
