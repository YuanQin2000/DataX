/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __XML_DATA_H__
#define __XML_DATA_H__

#include <cstring>

#include "Tracker/Trace.h"
#include "Common/Tree.h"
#include "Common/ForwardList.h"
#include "Memory/MemoryPool.h"
#include "Memory/LazyBuffer.h"

using std::strcmp;

class CXMLData
{
public:
    struct XMLAttribute {
        const char* pName;
        const char* pValue;

        XMLAttribute(const char* name = NULL, const char* value = NULL) :
            pName(name),
            pValue(value) {}
    };

    struct XMLElement {
        const char* pName;
        const char* pTextValue;
        CForwardList AttrList;

        XMLElement(CLazyBuffer& buffer, const char* name, const char* value = NULL) :
            pName(name),
            pTextValue(value),
            AttrList(&buffer) {}

        XMLElement(const char* name,
                   const char* value = NULL,
                   CForwardList* pList = NULL) :
            pName(name),
            pTextValue(value),
            AttrList()
            {
                if (pList) {
                    AttrList.ShallowCopy(*pList);
                    pList->Clear();
                }
            }

        bool operator==(const XMLElement& rhs)
        {
            return (this == &rhs || strcmp(pName, rhs.pName) == 0);
        }

        bool operator<(const XMLElement& rhs) const
        {
            return strcmp(pName, rhs.pName) < 0;
        }

#ifdef __DEBUG__
        void Dump(size_t depth);
#endif

        DISALLOW_DEFAULT_CONSTRUCTOR(XMLElement);
        DISALLOW_COPY_CONSTRUCTOR(XMLElement);
        DISALLOW_ASSIGN_OPERATOR(XMLElement);
    };

public:
    CXMLData() :
        m_Buffer(),
        m_Tree() {}
    ~CXMLData() {}

    CTree<XMLElement>& GetXMLTree() { return m_Tree; }

    XMLAttribute* CreateXMLAttribute(const char* pName, const char* pValue);
    XMLElement* CreateXMLElement(
        const char* pName,
        const char* pValue = NULL,
        CForwardList* pAttrList = NULL);
    bool SetXMLElementValue(XMLElement* pXMLElem, const char* pValue, size_t len);
    bool SetXMLElementValue(XMLElement* pXMLElem, const char* pValue)
    {
        return SetXMLElementValue(pXMLElem, pValue, strlen(pValue));
    }

    static void DestroyInstance(CXMLData* pInstance) { delete pInstance; }

private:
    CLazyBuffer m_Buffer;
    CTree<XMLElement> m_Tree;
};

#endif