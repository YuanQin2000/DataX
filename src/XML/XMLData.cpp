/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "XMLData.h"
#include <new>

#ifdef __DEBUG__
#include <cstdio>

void CXMLData::XMLElement::Dump(size_t depth)
{
    for (size_t i = 0; i < depth; ++i) {
        printf("\t");
    }
    printf("%s=>%s:", pName, pTextValue);
    CForwardList::Iterator iter = AttrList.Begin();
    CForwardList::Iterator iterEnd = AttrList.End();
    while (iter != iterEnd) {
        XMLAttribute* pAttr = reinterpret_cast<XMLAttribute*>(AttrList.DataAt(iter));
        printf("[%s=>%s]", pAttr->pName, pAttr->pValue);
        ++iter;
    }
    printf("\n");
}
#endif

CXMLData::XMLAttribute* CXMLData::CreateXMLAttribute(
    const char* pName, const char* pValue)
{
    ASSERT(pName);
    ASSERT(pValue);

    const char* pStoredName = m_Buffer.StoreString(pName);
    const char* pStoreValue = m_Buffer.StoreString(pValue);
    XMLAttribute* pResult = NULL;

    if (pStoredName && pStoreValue) {
        void* pMem = m_Buffer.Malloc(sizeof(XMLAttribute));
        if (pMem) {
            pResult = new (pMem) XMLAttribute(pStoredName, pStoreValue);
        }
    }
    return pResult;
}

CXMLData::XMLElement* CXMLData::CreateXMLElement(
    const char* pName,
    const char* pValue /* = NULL */,
    CForwardList* pAttrList /* = NULL */)
{
    ASSERT(pName);

    const char* pStoreValue = NULL;
    const char* pStoredName = m_Buffer.StoreString(pName);
    if (!pStoredName) {
        return NULL;
    }
    if (pValue) {
        pStoreValue = m_Buffer.StoreString(pValue);
        if (!pStoreValue) {
            return NULL;
        }
    }

    XMLElement* pResult = NULL;
    void* pMem = m_Buffer.Malloc(sizeof(XMLElement));
    if (pMem) {
        pResult = pAttrList ?
            new (pMem) XMLElement(pStoredName, pStoreValue, pAttrList) :
            new (pMem) XMLElement(m_Buffer, pStoredName, pStoreValue);
    }
    return pResult;
}

bool CXMLData::SetXMLElementValue(
    XMLElement* pXMLElem, const char* pValue, size_t len)
{
    ASSERT(pXMLElem);
    ASSERT(pValue);
    ASSERT(len > 0);

    const char* pStoreValue = m_Buffer.StoreNString(pValue, len);
    if (!pStoreValue) {
        OUTPUT_ERROR_TRACE("Store string to buffer failed\n");
        return false;
    }
    pXMLElem->pTextValue = pStoreValue;
    return true;
}
