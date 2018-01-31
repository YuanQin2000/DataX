/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/Macros.h"
#include "Common/Typedefs.h"
#include "Common/CharHelper.h"
#include "HTTPBase/HeaderField.h"
#include "HTTPBase/FieldValue.h"
#include "HTTP/HttpHeaderFieldDefs.h"
#include "Defines.h"
#include <cstdio>
#include <cstring>
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

using namespace std;

TEST_GROUP(HeaderField)
{
    CHeaderField* m_pHeaderField = NULL;
    CLazyBuffer m_Buffer;

void setup()
{
    m_pHeaderField = CHeaderField::CreateInstance(
        CHttpHeaderFieldDefs::GetRequestGlobalConfig(), m_Buffer);
}

void teardown()
{
    delete m_pHeaderField;
}

};

TEST(HeaderField, InsertValue)
{
    CHECK(m_pHeaderField != NULL);

    const char* pFieldName = "Host";
    const char* pFieldValue = "www.baidu.com";
    bool bRes = m_pHeaderField->SetFieldValue(pFieldName, pFieldValue);
    CHECK(bRes);
    CForwardList* pValueList = m_pHeaderField->GetFieldValueByName(pFieldName);
    CHECK(pValueList != NULL);
    LONGS_EQUAL(1, pValueList->Count());
    CForwardList::Iterator iter = pValueList->Begin();
    CForwardList::Iterator iterEnd = pValueList->End();
    while (iter != iterEnd) {
        IFieldValue* pValue = reinterpret_cast<IFieldValue*>(pValueList->DataAt(iter));
        CHECK(pValue != NULL);
        ++iter;
    }
}