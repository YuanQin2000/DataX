/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_REQUEST_PREF_H__
#define __HTTP_REQUEST_PREF_H__

#include "Config/ConfigObject.h"
#include "HTTPBase/HeaderField.h"

class CLazyBuffer;
class CHttpRequestPref : public IConfigObject
{
public:
    // From IConfigObject
    bool Initialize(tConfigNode* pConfigData);

    CHttpRequestPref(CLazyBuffer& buffer);
    ~CHttpRequestPref();

    CHeaderField::FieldInitedValue* GetFieldInitedValue() { return m_pFieldInitedValue; }
    bool Accept(const CHeaderField& respHeaderField);

private:
    void StoreConfig(tConfigNode* pConfigData,
                     CHeaderField::FieldInitedValue* pInitValue);
    void SetEncodingTypePreference(CHeaderField::FieldInitedValue* pInitedValues);

    CLazyBuffer& m_Buffer;
    CHeaderField::FieldInitedValue* m_pFieldInitedValue;    // Not Owned

    static const int s_PreferenceItems[];
};

#endif