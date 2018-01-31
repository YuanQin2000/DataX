/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HTTP/HttpCookieData.h"
#include "Common/ByteData.h"
#include "Defines.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(HttpCookieData)
{

void setup()
{
}

void teardown()
{
}

};


TEST(HttpCookieData, SerializeAndUnSerialize)
{
    CHttpCookieData::Attribute testAttr1("/", "Name", "Value", 1234567, false, false);
    CHttpCookieData::Attribute testAttr2("/", "ID", "Token", 0, false, true);
    CByteData serializedData;

    CByteData* pSerialized = CHttpCookieData::Serialize(&testAttr1, &serializedData);
    CHECK(pSerialized == &serializedData);

    CHttpCookieData::Attribute attr;
    bool bRes = CHttpCookieData::UnSerialize(pSerialized, &attr);
    CHECK(bRes);

    CHECK(testAttr1.bSecure == attr.bSecure);
    CHECK(testAttr1.bWildcardMatched == attr.bWildcardMatched);
    STRCMP_EQUAL(testAttr1.pName, attr.pName);
    STRCMP_EQUAL(testAttr1.pValue, attr.pValue);
    STRCMP_EQUAL(testAttr1.pPath, attr.pPath);
    LONGS_EQUAL(testAttr1.ExpireTime, attr.ExpireTime);
}
