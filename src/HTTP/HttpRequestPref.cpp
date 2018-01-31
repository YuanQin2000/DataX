/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpRequestPref.h"
#include <new>
#include "Common/Typedefs.h"
#include "HTTPBase/FieldValue.h"
#include "HttpHeaderFieldDefs.h"
#include "HttpToken.h"
#include "HttpTokenDefs.h"
#include "HttpUtils.h"
#include "Compress/CompressManager.h"
#include "Tracker/Trace.h"

const int CHttpRequestPref::s_PreferenceItems[] = {
    CHttpHeaderFieldDefs::REQ_FN_USER_AGENT,
    CHttpHeaderFieldDefs::REQ_FN_ACCEPT,
    CHttpHeaderFieldDefs::REQ_FN_ACCEPT_CHARSET,
    CHttpHeaderFieldDefs::REQ_FN_ACCEPT_ENCODING,
    CHttpHeaderFieldDefs::REQ_FN_ACCEPT_LANGUAGE,
};

CHttpRequestPref::CHttpRequestPref(CLazyBuffer& buffer) :
    IConfigObject(),
    m_Buffer(buffer),
    m_pFieldInitedValue(NULL)
{
}

CHttpRequestPref::~CHttpRequestPref()
{
}

bool CHttpRequestPref::Accept(const CHeaderField& respHeaderField)
{
    return true;
}

bool CHttpRequestPref::Initialize(tConfigNode* pConfigData)
{
    const CHeaderField::GlobalConfig* pReqConfig =
        CHttpHeaderFieldDefs::GetRequestGlobalConfig();

    CHeaderField::FieldInitedValue* pTemp = NULL;
    void* pMem =
        m_Buffer.Malloc(sizeof(CHeaderField::FieldInitedValue) +
            sizeof(CHeaderField::IndexValueHead) * COUNT_OF_ARRAY(s_PreferenceItems));
    if (pMem == NULL) {
        OUTPUT_ERROR_TRACE("Allocate memory failed\n");
        return false;
    }
    pTemp = new (pMem) CHeaderField::FieldInitedValue(m_Buffer, COUNT_OF_ARRAY(s_PreferenceItems));
    tConfigRoot::CDFSTraverser traverse(pConfigData);
    tConfigNode* pNode = traverse.GetNext();
    while (pNode) {
        ASSERT(traverse.GetDepth() == 1);
        CXMLData::XMLElement* pElem = pNode->GetElement();
        int fieldID = pReqConfig->GetFieldID(pElem->pName);
        ASSERT(fieldID >= 0);
        int index = -1;
        for (size_t i = 0; i < COUNT_OF_ARRAY(s_PreferenceItems); ++i) {
            if (s_PreferenceItems[i] == fieldID) {
                index = static_cast<int>(i);
                break;
            }
        }
        ASSERT(index >= 0);
        tValueCreator creatorFunc = pReqConfig->pItems[fieldID].CreatorFunction;
        ASSERT(creatorFunc);

        IFieldValue* pFieldValue = creatorFunc(
            pElem->pTextValue, strlen(pElem->pTextValue), m_Buffer);
        if (pFieldValue == NULL) {
            return false;
        }
        pTemp->Append(fieldID, pFieldValue);
        pNode = traverse.GetNext();
    }

    // Overwrite the Encoding Type since this is determined by implementation.
    SetEncodingTypePreference(pTemp);
    m_pFieldInitedValue = pTemp;
    return true;
}

void CHttpRequestPref::SetEncodingTypePreference(
    CHeaderField::FieldInitedValue* pInitedValues)
{
    TRACK_FUNCTION_LIFE_CYCLE;   
    ASSERT(pInitedValues);

    pInitedValues->Clear(CHttpHeaderFieldDefs::REQ_FN_ACCEPT_ENCODING);
    vector<CompressType>& supportTypes(
        CCompressManager::Instance()->GetSupportedDecompressType());
    vector<CompressType>::iterator iter = supportTypes.begin();
    vector<CompressType>::iterator iterEnd = supportTypes.end();
    while (iter != iterEnd) {
        void* pMem = m_Buffer.Malloc(sizeof(CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>));
        if (!pMem) {
            OUTPUT_ERROR_TRACE("Allocate Memory failed\n");
            return;
        }
        EncodingType et = NSHttpUtils::Compress2EncodingType(*iter);
        CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>* pObject =
            new (pMem) CTokenIndexFieldValue<CHttpTokenMap::CATEGORY_ENCODING>(et);
        pInitedValues->Append(CHttpHeaderFieldDefs::REQ_FN_ACCEPT_ENCODING, pObject);
        ++iter;
    }
}
