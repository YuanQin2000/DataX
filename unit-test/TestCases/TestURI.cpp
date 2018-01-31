/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Macros.h"
#include "URI/URI.h"
#include "URI/URIManager.h"
#include "URI/SchemeDefines.h"
#include "Defines.h"
#include <cstring>
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

using std::strlen;

TEST_GROUP(URI)
{
    struct AuthorityTestDesc {
        const char* pString;
        const char* pHostName;
        const char* pUser;
        unsigned short Port;
        bool bIPHost;
    };

    AuthorityTestDesc m_AuthorityTestData[5] = {
        { "www.google.com", "www.google.com", NULL, 0, false },
        { "www.google.com:443", "www.google.com", NULL, 443, false },
        { "yuanqin@google.com", "google.com", "yuanqin", 0, false },
        { "192.168.1.1:8080", "192.168.1.1", NULL, 8080, true },
        { "admin@192.168.1.1:8080", "192.168.1.1", "admin", 8080, true }
    };

void setup()
{
}

void teardown()
{
}

};


TEST(URI, TestAuthorityOK)
{
    char buffer[128];

    for (size_t i = 0; i < COUNT_OF_ARRAY(m_AuthorityTestData); ++i) {
        AuthorityTestDesc* pAuthorityExpected = &m_AuthorityTestData[i];
        CUriBuilder builder;
        CAuthority* pAuthority = builder.CreateAuthorityByString(pAuthorityExpected->pString);
        CHECK(pAuthority != NULL);
        CHECK(pAuthority->HostName().pName != NULL);
        bool bIsIP = true;
        bool bRes = CAuthority::IsValidHostname(pAuthority->HostName().pName, &bIsIP);
        CHECK(bRes);
        LONGS_EQUAL(pAuthorityExpected->bIPHost, bIsIP);

        STRCMP_EQUAL(pAuthorityExpected->pHostName, pAuthority->HostName().pName);

        if (pAuthorityExpected->pUser) {
            CHECK(pAuthority->UserName() != NULL);
            STRCMP_EQUAL(pAuthorityExpected->pUser, pAuthority->UserName());
            CHECK(CAuthority::IsValidUsername(pAuthorityExpected->pUser))
        } else {
            CHECK(pAuthority->UserName() == NULL);
        }

        LONGS_EQUAL(pAuthorityExpected->Port, pAuthority->GetPort(0));
        LONGS_EQUAL(pAuthorityExpected->bIPHost, pAuthority->HostName().bIsIP);

        size_t length = 0;
        bRes = pAuthority->Serialize(buffer, sizeof(buffer) - 1, URI_SERIALIZE_ALL, 0, &length);
        CHECK(bRes);
        buffer[length] = '\0';
        STRCMP_EQUAL(pAuthorityExpected->pString, buffer);

        delete pAuthority;
    }
}