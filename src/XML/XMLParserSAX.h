/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __XML_PARSER_SAX_H__
#define __XML_PARSER_SAX_H__

#include <set>
#include <memory>

#include "Common/Typedefs.h"
#include "XMLData.h"
#include "Common/Tree.h"
#include "Memory/LazyBuffer.h"

#include "expat/expat.h"

using std::set;
using std::shared_ptr;

class CXMLParserSAX
{
public:
    CXMLParserSAX();
    ~CXMLParserSAX();

    bool RegisterAnalyzedElement(const char* pName);

    bool ParseFile(const char* pFileName);
    bool Parse(const char* pBuffer, size_t length, bool bFinished);

    shared_ptr<CXMLData> GetXMLData() const
    {
        return m_pXMLData;
    }

private:
    bool NeedAnalyze(const char* pElemName) const
    {
        return m_AnalyzedElements.find(pElemName) != m_AnalyzedElements.end();
    }

    void SetupXMLParser(XML_Parser hParser);

    static void XMLCALL OnElementStart(
        void* pData, const char *pElem, const char **pAttrs);
    static void XMLCALL OnElementEnd(void* pData, const char *pElem);
    static void XMLCALL OnText(void* pData, const char *pBuf, int len);

private:
    XML_Parser m_hXMLParser;
    shared_ptr<CXMLData> m_pXMLData;
    CTree<CXMLData::XMLElement>::CNode* m_pCurrentNode;
    CTree<CXMLData::XMLElement>::CNode* m_pTopNode;
    bool m_bInAnalyzedElement;
    set<const char*, tStringCompareFunc> m_AnalyzedElements;
    CLazyBuffer m_StringBuffer;

    size_t m_CurTextIndex;
    char m_TextValueBuffer[1024];

    DISALLOW_COPY_CONSTRUCTOR(CXMLParserSAX);
    DISALLOW_ASSIGN_OPERATOR(CXMLParserSAX);
};

#endif