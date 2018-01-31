/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HttpConnectionPref.h"
#include <strings.h>
#include "HttpProxyPref.h"
#include "Tracker/Trace.h"

CHttpConnectionPref::CHttpConnectionPref(CLazyBuffer& buffer) :
    IConfigObject(),
    m_Buffer(buffer),
    m_pHttpProxy(NULL),
    m_SendBufferSize(4096),
    m_RecvBufferSize(8192)
{
}

CHttpConnectionPref::~CHttpConnectionPref()
{
    delete m_pHttpProxy;
}

///////////////////////////////////////////////////////////////////////////////
//
// TODO: Following functions should be replaced by XML auto descerializer.
//
///////////////////////////////////////////////////////////////////////////////
bool CHttpConnectionPref::Initialize(tConfigNode* pConfigData)
{
    CXMLData::XMLElement* pElem = pConfigData->GetElement();

    if (!AnalyzeGlobalAttribution(pElem->AttrList)) {
        return false;
    }

    bool bRes = false;
    tConfigRoot::CDFSTraverser traverse(pConfigData);
    tConfigNode* pNode = traverse.GetNext();
    while (pNode) {
        pElem = pNode->GetElement();
        if (strcasecmp(pElem->pName, "proxy") == 0) {
            ASSERT(m_pHttpProxy == NULL);
            m_pHttpProxy = CHttpProxyPref::CreateInstance(pNode, m_Buffer);
            if (m_pHttpProxy == NULL) {
                OUTPUT_WARNING_TRACE("Create CHttpProxyPref failed\n");
                bRes = false;
            }
        }
        if (!bRes) {
            break;
        }
        pNode = traverse.GetNext();
    }
    return bRes;
}

bool CHttpConnectionPref::AnalyzeGlobalAttribution(CForwardList& attrList)
{
    CForwardList::Iterator iter = attrList.Begin();
    CForwardList::Iterator iterEnd = attrList.End();
    while (iter != iterEnd) {
        CXMLData::XMLAttribute* pAttr =
            reinterpret_cast<CXMLData::XMLAttribute*>(attrList.DataAt(iter));
        if (pAttr->pName == NULL || pAttr->pValue == NULL) {
            break;
        }
        if (strcasecmp(pAttr->pName, "send-buffer-bytes") == 0) {
            m_SendBufferSize = atoi(pAttr->pValue);
        } else if (strcasecmp(pAttr->pName, "receive-buffer-bytes") == 0) {
            m_RecvBufferSize = atoi(pAttr->pValue);
        } else {
            break;
        }
        ++iter;
    }
    return iter == iterEnd;
}
