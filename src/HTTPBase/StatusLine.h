/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_STATUS_LINE_H__
#define __HTTP_BASE_STATUS_LINE_H__

#include "Common/Typedefs.h"
#include "Token.h"

class CLazyBuffer;
class CStatusLine
{
public:
    CStatusLine(
        tTokenID version, int code, char* pPhrase, bool bTransferOwnship = false);
    ~CStatusLine();

    bool Print(char* pBuffer, size_t len, size_t* pOutPrintLen);

    tTokenID GetVersionID() const { return m_VersionID; }
    int GetStatusCode() const { return m_StatusCode; }
    const char* GetStatusPhrase() const { return m_pPhrase; }

    static CStatusLine* CreateInstance(const char* pString, size_t len, CLazyBuffer* pBuffer);
    static void* CreateInstance1(const char* pString, size_t len, CLazyBuffer* pBuffer)
    {
        return CreateInstance(pString, len, pBuffer);
    }

private:
    tTokenID m_VersionID;
    int m_StatusCode;
    char* m_pPhrase;
    bool m_bOwnPhrase;
};

#endif