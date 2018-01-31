/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Network/Domain.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"
#include <cstring>

using std::strlen;

TEST_GROUP(NetworkDomain)
{

void setup()
{
    // memory leak detected caused by STL
    MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
}

void teardown()
{
    MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
}

};

TEST(NetworkDomain, ParseStringOK)
{
    SETUP_MEMORY_LEAK_CHECK;
    NSInternetDomain::DomainDesc desc;

    bool bRes = NSInternetDomain::ParseString("baidu.com", &desc);
    CHECK(bRes);

    bRes = NSInternetDomain::ParseString("www.baidu.com", &desc);
    CHECK(bRes);

    bRes = NSInternetDomain::ParseString("www.baidu.com.cn", &desc);
    CHECK(bRes);

    bRes = NSInternetDomain::ParseString("pan.baidu.cn", &desc);
    CHECK(bRes);
}

TEST(NetworkDomain, ParseStringFailed)
{
    SETUP_MEMORY_LEAK_CHECK;
    NSInternetDomain::DomainDesc desc;
    bool bRes = NSInternetDomain::ParseString("www..baidu.com", &desc);
    CHECK(!bRes);
    bRes = NSInternetDomain::ParseString("www.bai_du.com", &desc);
    CHECK(!bRes);
    bRes = NSInternetDomain::ParseString("www.baidu.com.aa", &desc);
    CHECK(!bRes);
    bRes = NSInternetDomain::ParseString("www.baidu.comm", &desc);
    CHECK(!bRes);
}

TEST(NetworkDomain, MatchOK)
{
    SETUP_MEMORY_LEAK_CHECK;
    NSInternetDomain::DomainDesc desc;
    bool bRes = NSInternetDomain::ParseString("baidu.com", &desc);
    CHECK(bRes);
    CHECK(NSInternetDomain::IsMatch(&desc, "baidu.com"));
    CHECK(NSInternetDomain::IsMatch(&desc, "www.baidu.com"));

    bRes = NSInternetDomain::ParseString("www.baidu.com", &desc);
    CHECK(bRes);
    CHECK(NSInternetDomain::IsMatch(&desc, "xxx.www.baidu.com"));

    bRes = NSInternetDomain::ParseString("www.baidu.com.cn", &desc);
    CHECK(bRes);
    CHECK(NSInternetDomain::IsMatch(&desc, "xxx.www.baidu.com.cn"));

    bRes = NSInternetDomain::ParseString("pan.baidu.cn", &desc);
    CHECK(bRes);
    CHECK(NSInternetDomain::IsMatch(&desc, "i.pan.baidu.cn"));
}


TEST(NetworkDomain, MatchFailed)
{
    SETUP_MEMORY_LEAK_CHECK;
    NSInternetDomain::DomainDesc desc;
    bool bRes = NSInternetDomain::ParseString("baidu.com", &desc);
    CHECK(bRes);
    CHECK(!NSInternetDomain::IsMatch(&desc, "baidu.cn"));

    bRes = NSInternetDomain::ParseString("www.baidu.com", &desc);
    CHECK(bRes);
    CHECK(!NSInternetDomain::IsMatch(&desc, "pan.baidu.com"));

    bRes = NSInternetDomain::ParseString("www.baidu.com.cn", &desc);
    CHECK(bRes);
    CHECK(!NSInternetDomain::IsMatch(&desc, "baidu.com"));

    bRes = NSInternetDomain::ParseString("pan.baidu.cn", &desc);
    CHECK(bRes);
    CHECK(!NSInternetDomain::IsMatch(&desc, "xxx.www.bai-du.com.cn"));
    CHECK(!NSInternetDomain::IsMatch(&desc, "an.baidu.cn"));
}

TEST(NetworkDomain, DomainSectionAccess)
{
    SETUP_MEMORY_LEAK_CHECK;
    NSInternetDomain::CDomainSectionAccess access("pan.bbs.baidu.com");
    const char* pRegisterDomain = access.RegisteredDomain();

    STRCMP_EQUAL("baidu.com", pRegisterDomain);

    static const char* sectionIndex[] = {
        "pan.bbs.baidu.com",
        "bbs.baidu.com",
        "baidu.com",
        "com"
    };

    int idx = 0;
    for (const char* pCurrent = access.Begin(); pCurrent; pCurrent = access.Next()) {
        STRCMP_EQUAL(sectionIndex[idx], pCurrent);
        CHECK(strlen(pCurrent) == access.CurrentLength());
        if (idx < 2) {
            CHECK(pCurrent < pRegisterDomain);
        } else if (idx == 2) {
            CHECK(pCurrent == pRegisterDomain);
        } else {
            CHECK(pCurrent > pRegisterDomain);
        }
        ++idx;
    }
    CHECK(idx == 4);
}
