/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CliCmdHelper.h"
#include "Common/Macros.h"
#include "Common/StringIndex.h"
#include "Tracker/Trace.h"
#include "Common/CharHelper.h"

using std::pair;

///////////////////////////////////////////////////////////////////////////////
//
// CCliCmdHelperManager Implementation
//
///////////////////////////////////////////////////////////////////////////////
CCliCmdHelperManager::CCliCmdHelperManager() :
    m_CliMsgHelpers{0},
    m_CliHelperNames(NSCharHelper::StringCaseCompare),
    m_Root(&m_RootElem),
    m_RootElem(CCommandTree::TYPE_ROOT, 0, NULL),
    m_CommandInfoMsg(),
    m_bCreateMsg(false)
{
}

CCliCmdHelperManager::~CCliCmdHelperManager()
{
    ReleaseInfoMsg(m_CommandInfoMsg);
}

bool CCliCmdHelperManager::RegisterCliMsgHelper(ICliCommandHelper* pHelper)
{
    ASSERT(pHelper);

    if (m_CliHelperNames.size() >= COUNT_OF_ARRAY(m_CliMsgHelpers)) {
        OUTPUT_ERROR_TRACE("Too many command helpers\n");
        return false;
    }
    const char* pIndexName = pHelper->GetCommandName();
    pair<set<const char*>::iterator, bool> res = m_CliHelperNames.insert(pIndexName);
    if (!res.second) {
        return false;
    }

    bool bRes = false;
    for (size_t i = 0; i < COUNT_OF_ARRAY(m_CliMsgHelpers); ++i) {
        if (m_CliMsgHelpers[i].pHelper == NULL) {
            m_CliMsgHelpers[i].pHelper = pHelper;
            m_CliMsgHelpers[i].Index = i;

            CCommandTree::tInfoNode* pNode = pHelper->GetHint();
            ASSERT(pNode);  // If a helper registered it must provide the command tree.

            CCommandTree::InfoElement* pElem = pNode->GetElement();
            ASSERT(pElem);
            ASSERT(pElem->Type == CCommandTree::TYPE_COMMAND);
            ASSERT(pElem->pItemData);
            CCommandTree::CommandItem* pCmdItem =
                reinterpret_cast<CCommandTree::CommandItem*>(pElem->pItemData);
            pCmdItem->CmdID = i;
            m_Root.AddChild(pNode);
            ++m_RootElem.SubCount;
            bRes = true;
            break;
        }
    }
    return bRes;
}

const CCliCmdHelperManager::tHintMsgVector& CCliCmdHelperManager::GetHint()
{
    if (m_bCreateMsg) {
        return m_CommandInfoMsg;
    }

#ifdef __DEBUG__
    printf("---------- Command Info Tree Dump ----------\n");
    m_Root.Dump();
    printf("--------------------------------------------\n");
#endif

    m_bCreateMsg = true;
    CCommandTreeSerializer serializer(m_Root);
    NSCliMsg::InfoPayload* pInfoPayload = serializer.Serialize();
    while (pInfoPayload) {
        size_t length = pInfoPayload->GetLength();
        NSCliMsg::InfoPayload* pDup =
            reinterpret_cast<NSCliMsg::InfoPayload*>(malloc(length));
        if (pDup == NULL) {
            ReleaseInfoMsg(m_CommandInfoMsg);
            OUTPUT_ERROR_TRACE("Allocate memory failed\n");
            break;
        }
        memcpy(pDup, pInfoPayload, length);
        m_CommandInfoMsg.push_back(pDup);

#ifdef __DEBUG__
        OUTPUT_DEBUG_TRACE("Information block count: %d, length: %d\n", pInfoPayload->Count, length);
        DumpBytes(pInfoPayload, length);
#endif

        pInfoPayload = serializer.Serialize();
    }
    return m_CommandInfoMsg;
}

void CCliCmdHelperManager::ReleaseInfoMsg(tHintMsgVector& msgVector)
{
    tHintMsgVector::iterator iter = msgVector.begin();
    tHintMsgVector::iterator iterEnd = msgVector.end();
    while (iter != iterEnd) {
        NSCliMsg::InfoPayload* pInfo = *iter;
        free(pInfo);
        ++iter;
    }
    msgVector.clear();
}

void CCliCmdHelperManager::ExecuteCommand(
    uint16_t sessionID,
    CVector& command,
    IResultHandler& resultHandler)
{
    NSCliMsg::CommandDataBlock* pCmdBlock =
        reinterpret_cast<NSCliMsg::CommandDataBlock*>(command.At(0));
    if (pCmdBlock->Type != NSCliMsg::BT_COMMAND ||
        pCmdBlock->CmdID >= COUNT_OF_ARRAY(m_CliMsgHelpers)) {
        resultHandler.OnResult(sessionID, NSCliMsg::MSC_PARAMETER_INVALID);
        return;
    }

    ICliCommandHelper* pHelper = m_CliMsgHelpers[pCmdBlock->CmdID].pHelper;
    if (pHelper == NULL) {
        resultHandler.OnResult(sessionID, NSCliMsg::MSC_COMMAND_NOT_FOUND);
        return;
    }

    command.PopFront();
    pHelper->ExecuteCommand(sessionID, command, resultHandler);
}


///////////////////////////////////////////////////////////////////////////////
//
// CCliCmdHelperRegister Implementation
//
///////////////////////////////////////////////////////////////////////////////
CCliCmdHelperRegister::CCliCmdHelperRegister(ICliCommandHelper* pHelper) :
    m_pHelper(pHelper)
{
    if (pHelper) {
        CCliCmdHelperManager::Instance()->RegisterCliMsgHelper(pHelper);
    }
}

CCliCmdHelperRegister::~CCliCmdHelperRegister()
{
}
