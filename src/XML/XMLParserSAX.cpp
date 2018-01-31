/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "XMLParserSAX.h"
#include "XMLData.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"
#include <cstdio>
#include <cstring>
#include <errno.h>


CXMLParserSAX::CXMLParserSAX() :
    m_hXMLParser(NULL),
    m_pXMLData(NULL),
    m_pCurrentNode(NULL),
    m_pTopNode(NULL),
    m_bInAnalyzedElement(false),
    m_AnalyzedElements(NSCharHelper::StringCompare),
    m_StringBuffer(),
    m_CurTextIndex(0)
{
}

CXMLParserSAX::~CXMLParserSAX()
{
    if (m_hXMLParser) {
        XML_ParserFree(m_hXMLParser);
    }
}

bool CXMLParserSAX::RegisterAnalyzedElement(const char* pName)
{
    ASSERT(pName);

    const char* pStoredName = m_StringBuffer.StoreString(pName);
    if (pStoredName) {
        m_AnalyzedElements.insert(pStoredName);
        return true;
    }
    return false;
}

bool CXMLParserSAX::ParseFile(const char* pFileName)
{
    ASSERT(pFileName);

    FILE* pFile = NULL;
    char* pFileBuffer = NULL;
    XML_Parser hParser = NULL;
    size_t len = 0;
    bool bEof = false;
    bool bRes = false;

    pFile = fopen(pFileName, "r");
    if (!pFile) {
        OUTPUT_ERROR_TRACE("fopen %s: %s\n", pFileName, strerror(errno));
        goto EXIT;
    }

    pFileBuffer = new char[4096];
    if (!pFileBuffer) {
        OUTPUT_ERROR_TRACE("new memory failed\n");
        goto EXIT;
    }

    hParser = XML_ParserCreate(NULL);
    if (!hParser) {
        goto EXIT;
    }

    SetupXMLParser(hParser);
    m_hXMLParser = hParser;

    if (!m_pXMLData.get()) {
        CXMLData* pData = new CXMLData();
        if (!pData) {
            goto EXIT;
        }
        m_pXMLData.reset(pData, CXMLData::DestroyInstance);
    }

    do {
        len = (int)fread(pFileBuffer, 1, sizeof(pFileBuffer), pFile);
        if (ferror(pFile)) {
            goto EXIT;
        }
        bEof = feof(pFile);
        bRes = Parse(pFileBuffer, len, bEof);
    } while (!bEof && bRes);

EXIT:
    if (pFile) {
        fclose(pFile);
    }
    if (pFileBuffer) {
        delete [] pFileBuffer;
    }
    if (hParser) {
        XML_ParserFree(hParser);
        if (m_hXMLParser == hParser) {
            m_hXMLParser = NULL;
        }
    }
    return bRes;
}

bool CXMLParserSAX::Parse(const char* pBuffer, size_t length, bool bFinished)
{
    bool bCreateParser = false;

    if (!m_pXMLData.get()) {
        CXMLData* pData = new CXMLData();
        if (!pData) {
            return false;
        }
        m_pXMLData.reset(pData, CXMLData::DestroyInstance);
    }

    if (!m_hXMLParser) {
        m_hXMLParser = XML_ParserCreate(NULL);
        if (!m_hXMLParser) {
            return false;
        }
        bCreateParser = true;
        SetupXMLParser(m_hXMLParser);
    }

    bool bRes = true;
    if (XML_Parse(m_hXMLParser, pBuffer, length, bFinished) == XML_STATUS_ERROR) {
        OUTPUT_ERROR_TRACE("Parse error at line %lu:\n%s\n",
                XML_GetCurrentLineNumber(m_hXMLParser),
                XML_ErrorString(XML_GetErrorCode(m_hXMLParser)));
        bRes = false;
    }
    if (bCreateParser) {
        XML_ParserFree(m_hXMLParser);
        m_hXMLParser = NULL;
    }
    return bRes;
}

void CXMLParserSAX::SetupXMLParser(XML_Parser hParser)
{
    XML_SetUserData(hParser, this);
    XML_SetElementHandler(hParser, CXMLParserSAX::OnElementStart, CXMLParserSAX::OnElementEnd);
    XML_SetCharacterDataHandler(hParser, CXMLParserSAX::OnText);
}

void XMLCALL CXMLParserSAX::OnElementStart(
    void* pData, const char *pElem, const char **pAttrs)
{
    CXMLParserSAX* pThis = reinterpret_cast<CXMLParserSAX*>(pData);
    ASSERT(pThis->m_CurTextIndex == 0);

    CXMLData* pXML = pThis->m_pXMLData.get();
    CXMLData::XMLElement* pXMLElem = pXML->CreateXMLElement(pElem);
    if (pXMLElem == NULL) {
        return;
    }

    CTree<CXMLData::XMLElement>& tree = pXML->GetXMLTree();
    if (pThis->m_pCurrentNode) {
        pThis->m_pCurrentNode = tree.AddChild(pThis->m_pCurrentNode, pXMLElem);
    } else {
        ASSERT(tree.GetRoot() == NULL);
        pThis->m_pCurrentNode = tree.CreateRoot(pXMLElem);
    }

    if (pThis->m_bInAnalyzedElement || pThis->NeedAnalyze(pElem)) {
        if (!pThis->m_bInAnalyzedElement) {
            pThis->m_bInAnalyzedElement = true;
            pThis->m_pTopNode = pThis->m_pCurrentNode;
        }
        for (int i = 0; pAttrs[i]; i += 2) {
            CXMLData::XMLAttribute* pAttr =
                pXML->CreateXMLAttribute(pAttrs[i], pAttrs[i + 1]);
            if (pAttr) {
                pXMLElem->AttrList.PushBack(pAttr);
            }
        }
    }
}

void XMLCALL CXMLParserSAX::OnElementEnd(void* pData, const char *pElem)
{
    ASSERT(pData);

    CXMLParserSAX* pThis = reinterpret_cast<CXMLParserSAX*>(pData);
    if (pThis->m_CurTextIndex > 0) {
        const char* pStrEnd = NSCharHelper::TrimRightSpace(
            pThis->m_TextValueBuffer, pThis->m_TextValueBuffer + pThis->m_CurTextIndex + 1);
        CXMLData::XMLElement* pCurrentElement = pThis->m_pCurrentNode->GetElement();
        pThis->m_pXMLData.get()->SetXMLElementValue(
            pCurrentElement, pThis->m_TextValueBuffer, pStrEnd - pThis->m_TextValueBuffer);
        pThis->m_CurTextIndex = 0;
    }

    if (pThis->m_pTopNode == pThis->m_pCurrentNode) {
        pThis->m_bInAnalyzedElement = false;
        pThis->m_pTopNode = NULL;
    }
    if (pThis->m_pCurrentNode) {
        pThis->m_pCurrentNode = pThis->m_pCurrentNode->GetParent();
    }
}

void XMLCALL CXMLParserSAX::OnText(void* pData, const char *pBuf, int len)
{
    CXMLParserSAX* pThis = reinterpret_cast<CXMLParserSAX*>(pData);
    if (pThis->m_bInAnalyzedElement &&
        pThis->m_CurTextIndex < sizeof(pThis->m_TextValueBuffer)) {
        const char* pString = pBuf;
        const char* pEnd = pBuf + len;
        if (pThis->m_CurTextIndex == 0) {
            pString = NSCharHelper::TrimLeftSpace(pBuf, pEnd);
            if (pString == NULL) {
                return;
            }
        }
        size_t textLen = pEnd - pString;
        memcpy(&pThis->m_TextValueBuffer[pThis->m_CurTextIndex], pString, textLen);
        pThis->m_CurTextIndex += textLen;
    }
}
