/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpCookieData.h"
#include <cstdlib>
#include "Common/Macros.h"
#include "Common/ByteData.h"
#include "Common/CharHelper.h"

using std::malloc;
using std::free;

///////////////////////////////////////////////////////////////////////////////
//
//    variable length      variable length     variable len  8 Bytes(64b) 1 Byte
// +--------------------+---------------------+-------------+-------------+-------+
// | Cookie Name String | Cookie Value String | Path String | Expire time | Flags |
// +--------------------+---------------------+-------------+-------------+-------+
//
// Flags:
// 00000001: Wildcard Host Name Match
// 00000010: Security only.
//
///////////////////////////////////////////////////////////////////////////////
CByteData* CHttpCookieData::Serialize(
    Attribute* pCookieAttr, CByteData* pOutData)
{
    size_t nameLen = strlen(pCookieAttr->pName) + 1;
    size_t valueLen = strlen(pCookieAttr->pValue) + 1;
    size_t pathLen = strlen(pCookieAttr->pPath) + 1;
    size_t totalLen =
        nameLen + valueLen + pathLen + sizeof(tSecondTick) + sizeof(uint8_t);
    char* pBuffer = reinterpret_cast<char*>(malloc(totalLen));
    if (pBuffer == NULL) {
        OUTPUT_ERROR_TRACE("malloc failed.\n");
        return NULL;
    }
    char* pCur = pBuffer;
    memcpy(pCur, pCookieAttr->pName, nameLen);
    pCur += nameLen;
    memcpy(pCur, pCookieAttr->pValue, valueLen);
    pCur += valueLen;
    memcpy(pCur, pCookieAttr->pPath, pathLen);
    pCur += pathLen;

    // Handle the expired time.
    tSecondTick expireTime = pCookieAttr->ExpireTime;
    if (expireTime == 0) {
        time_t now;
        time(&now);
        expireTime = COOKIE_DEFAULT_MAX_AGE + now;
    }
    expireTime = HOST2BE64(expireTime);
    memcpy(pCur, &expireTime, sizeof(expireTime));
    pCur += sizeof(expireTime);

    // Handle the flags.
    uint8_t bitSet = 0;
    if (pCookieAttr->bWildcardMatched) {
        SET_FLAG(bitSet, WILD_CARD_FLAG);
    }
    if (pCookieAttr->bSecure) {
        SET_FLAG(bitSet, SECURE_FLAG);
    }
    *pCur = bitSet;

    pOutData->SetData(pBuffer, totalLen, free);
    return pOutData;
}

bool CHttpCookieData::UnSerialize(
    CByteData* pData, Attribute* pOutCookieAttr)
{
    char* pCur = reinterpret_cast<char*>(pData->GetData());
    size_t dataLen = pData->GetLength();

    // Cookie name
    size_t len = NSCharHelper::StringNLength(pCur, dataLen);
    if (len == 0 || len == dataLen) {
        return false;
    }
    dataLen -= len;
    pOutCookieAttr->pName = pCur;
    pCur += len;

    // Cookie value
    len = NSCharHelper::StringNLength(pCur, dataLen);
    if (len == 0 || len == dataLen) {
        return false;
    }
    dataLen -= len;
    pOutCookieAttr->pValue = pCur;
    pCur += len;

    // Path
    len = NSCharHelper::StringNLength(pCur, dataLen);
    if (len == 0 || len == dataLen) {
        return false;
    }
    dataLen -= len;
    pOutCookieAttr->pPath = pCur;
    pCur += len;

    if (dataLen != sizeof(tSecondTick) + sizeof(uint8_t)) {
        return false;
    }

    // expired time.
    tSecondTick etBE64 = *(reinterpret_cast<tSecondTick*>(pCur));
    pOutCookieAttr->ExpireTime = BE2HOST64(etBE64);
    pCur += sizeof(tSecondTick);

    // Flags
    uint8_t bitSet = *pCur;
    pOutCookieAttr->bSecure = TEST_FLAG(bitSet, SECURE_FLAG);
    pOutCookieAttr->bWildcardMatched = TEST_FLAG(bitSet, WILD_CARD_FLAG);
    return true;
}
