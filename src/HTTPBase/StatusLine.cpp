/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "StatusLine.h"

#include <cstdio>
#include <cstring>
#include <new>

#include "Tracker/Trace.h"
#include "Token.h"
#include "TokenDefs.h"
#include "Memory/LazyBuffer.h"
#include "Common/CharHelper.h"

using std::snprintf;
using std::strchr;
using std::memcpy;

CStatusLine::CStatusLine(
    tTokenID version,
    int code,
    char* pPhrase,
    bool bTransferOwnship /* = false */) :
    m_VersionID(version),
    m_StatusCode(code),
    m_pPhrase(pPhrase),
    m_bOwnPhrase(bTransferOwnship)
{
    ASSERT(CTokenManager::Instance()->CheckValidity(TOKEN_PROTOCOL_VERSION, version));
    ASSERT(code > 0);
}

CStatusLine::~CStatusLine()
{
    if (m_bOwnPhrase && m_pPhrase) {
        free(m_pPhrase);
    }
}

CStatusLine* CStatusLine::CreateInstance(
    const char* pString, size_t len, CLazyBuffer* pBuffer /* = NULL */)
{
    ASSERT(len > 0);

    tTokenID versionID;
    const char* pPhrase;
    size_t phraseLen;
    const char* pCode;
    int code;

    const char* pEnd = pString + len;
    const char* pSpace = strchr(pString, ' ');    

    if (!pSpace) {
        return NULL;
    }
    versionID = CTokenManager::Instance()->GetTokenID(
        TOKEN_PROTOCOL_VERSION, pString, pSpace - pString);
    if (versionID == INVALID_TOKEN_ID) {
        return NULL;
    }

    pCode = pSpace + 1;
    pSpace = strchr(pCode, ' ');
    if (!pSpace) {
        return NULL;
    }
    if (!NSCharHelper::GetIntByString(pCode, pSpace - pCode, &code)) {
        return NULL;
    }

    pPhrase = NSCharHelper::TrimLeftSpace(pSpace + 1, pEnd);
    phraseLen = pEnd - pPhrase;

    if (pBuffer == NULL) {
        char* pPhraseBuf;
        if (pPhrase) {
            pPhraseBuf = reinterpret_cast<char*>(malloc(phraseLen + 1));
            memcpy(pPhraseBuf, pPhrase, phraseLen);
            pPhraseBuf[phraseLen] = '\0';
            return new CStatusLine(versionID, code, pPhraseBuf, true);
        }
        return NULL;
    }

    uint8_t* pMem = NULL;
    char* pPhraseStr = NULL;
    if (pPhrase) {
        pMem = reinterpret_cast<uint8_t*>(
            pBuffer->Malloc(sizeof(CStatusLine) + phraseLen + 1));
        if (pMem == NULL) {
            return NULL;
        }
        pPhraseStr = reinterpret_cast<char*>(pMem + sizeof(CStatusLine));
        memcpy(pPhraseStr, pPhrase, phraseLen);
        pMem[phraseLen] = '\0';
    } else {
        pMem = reinterpret_cast<uint8_t*>(pBuffer->Malloc(sizeof(CStatusLine)));
        if (!pMem) {
            return NULL;
        }
    }

    return new (pMem) CStatusLine(versionID, code, pPhraseStr, false);
}

bool CStatusLine::Print(char* pBuffer, size_t len, size_t* pOutPrintLen)
{
    ASSERT(len > 0);

    const char* pVersion =
        CTokenManager::Instance()->GetTokenString(TOKEN_PROTOCOL_VERSION, m_VersionID);
    const char* pPhrase = m_pPhrase ? m_pPhrase : " ";
    int printLen = snprintf(pBuffer, len, "%s %d %s\r\n", pVersion, m_StatusCode, pPhrase);

    ASSERT(printLen > 0);

    bool bFinished = false;
    char last = pBuffer[printLen - 1];
    if (last == '\0' || last == '\n') {
        bFinished = true;
    }
    *pOutPrintLen = static_cast<size_t>(printLen);
    return bFinished;
}
