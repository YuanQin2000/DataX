/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CommandTree.h"
#include <memory>
#include <cstdlib>
#include <cstring>
#include "SessionManager.h"
#include "SendRecv.h"
#include "IO/IOContext.h"
#include "Tracker/Trace.h"

#ifdef __DEBUG__
#include <stdio.h>
#endif

using std::strlen;
using std::malloc;
using std::free;

CCommandTree::CCommandTree(CIOContext& io) :
    m_IO(io), m_pInfoTree(NULL), m_Buffer()
{
}

CCommandTree::~CCommandTree()
{
    delete m_pInfoTree;
}

CCommandTree* CCommandTree::CreateInstance(CIOContext& io)
{
    CCommandTree* pInstance = new CCommandTree(io);
    if (pInstance) {
        pInstance->m_pInfoTree = pInstance->CreateInfoTree();
        if (pInstance->m_pInfoTree) {

#ifdef __DEBUG__
            printf("---------- Command Info Tree Dump ----------\n");
            pInstance->m_pInfoTree->Dump();
            printf("--------------------------------------------\n");
#endif
        } else {
            delete pInstance;
            pInstance = NULL;
        }
    }
    return pInstance;
}

CCommandTree::tInfoTree* CCommandTree::CreateInfoTree()
{
    uint8_t* pBuffer = reinterpret_cast<uint8_t*>(malloc(NSCliMsg::MAX_PACKET_LENGTH));
    if (pBuffer == NULL) {
        return NULL;
    }

    uint16_t sessionID = CSessionManager::Instance()->NewSesssion();
    NSCliMsg::Message msg(NSCliMsg::MSG_IDENTIFIER_QUERY, sessionID, false);
    if (!NSSendRecv::Send(&msg, m_IO)) {
        OUTPUT_ERROR_TRACE("Send Message failed\n");
        return NULL;
    }

    tInfoTree* pTree = NULL;
    CCommandTreeBuilder builder(m_Buffer);
    do {
        NSCliMsg::Message* pRecvMsg =
            NSSendRecv::Recv(sessionID, m_IO, pBuffer, NSCliMsg::MAX_PACKET_LENGTH);
        if (pRecvMsg == NULL ||
            pRecvMsg->Identifier != NSCliMsg::MSG_IDENTIFIER_INFO) {
            break;
        }
        NSCliMsg::InfoPayload* pData =
            reinterpret_cast<NSCliMsg::InfoPayload*>(pRecvMsg->Payload);
        if (!builder.AppendData(pData)) {
            break;
        }
        if (builder.IsFinished()) {
            pTree = builder.CreateInfoTree();
            break;
        }
    } while (true);

    return pTree;
}

CCommandTree::tInfoNode*
CCommandTree::FindCommandNode(tInfoNode* pNode, const char* pString)
{
    ASSERT(pNode);
    ASSERT(pString);

    tInfoNode* pResult = NULL;
    for (tInfoNode* pCur = pNode->GetChild(); pCur; pCur = pCur->GetSlibing()) {
        InfoElement* pElem = pCur->GetElement();
        const char* pName = pElem->GetName();
        ASSERT(pName);
        if (strcasecmp(pName, pString) == 0) {
            pResult = pCur;
            break;
        }
    }
    return pResult;
}

CCommandTree::tInfoNode*
CCommandTree::FindVariableNode(tInfoNode* pNode, const char* pString /* = NULL */)
{
    ASSERT(pNode);

    tInfoNode* pResult = NULL;
    for (tInfoNode* pCur = pNode->GetChild(); pCur; pCur = pCur->GetSlibing()) {
        InfoElement* pElem = pCur->GetElement();
        if (pElem->Type == TYPE_VARIABLE) {
            if (pString == NULL) {
                pResult = pCur;
                break;
            } else {
                char* pName = reinterpret_cast<VariableItem*>(pElem->pItemData)->Name;
                if (strcasecmp(pName, pString) == 0) {
                    pResult = pCur;
                    break;
                }
            }
        }
    }
    return pResult;
}

CCommandTree::tInfoNode* CCommandTree::FindFitNode(tInfoNode* pNode, const char* pString)
{
    ASSERT(pNode);
    ASSERT(pString);

    tInfoNode* pResult = NULL;
    for (tInfoNode* pCur = pNode->GetChild(); pCur; pCur = pCur->GetSlibing()) {
        InfoElement* pElem = pCur->GetElement();
        const char* pName = pElem->GetName();
        ASSERT(pName);
        if (strcasecmp(pName, pString) == 0) {
            pResult = pCur;
            break;
        }
        if (pElem->Type == TYPE_VARIABLE) {
            pResult = pCur;
        }
    }
    return pResult;
}

#ifdef __DEBUG__
void CCommandTree::InfoElement::Dump()
{
    switch (Type) {
    case TYPE_ROOT:
        printf("Type: ROOT, Sub Count: %d\n", SubCount);
        break;
    case TYPE_COMMAND:
        printf("Type: COMMAND, Sub Count: %d, Command ID: %d, Command Name: %s\n",
                SubCount,
                reinterpret_cast<CommandItem*>(pItemData)->CmdID,
                reinterpret_cast<CommandItem*>(pItemData)->Name);
        break;
    case TYPE_VARIABLE:
        printf("Type: VARIABLE, Sub Count: %d, Char String: %s, Mandatory: %s, Variable Name: %s\n",
                SubCount,
                reinterpret_cast<VariableItem*>(pItemData)->bCharString ? "YES" : "NO",
                reinterpret_cast<VariableItem*>(pItemData)->bMandatory ? "YES" : "NO",
                reinterpret_cast<VariableItem*>(pItemData)->Name);
        break;
    default:
        ASSERT(false);
        break;
    }
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CCommandTreeBuilder Implementation
//
///////////////////////////////////////////////////////////////////////////////
CCommandTreeBuilder::CCommandTreeBuilder(CLazyBuffer& buffer) :
    m_pRoot(NULL),
    m_pCurrent(NULL),
    m_Buffer(buffer),
    m_bFinished(false),
    m_CurrentCount(0)
{
}

CCommandTreeBuilder::~CCommandTreeBuilder()
{
    delete m_pRoot;
}

CCommandTree::tInfoTree* CCommandTreeBuilder::CreateInfoTree()
{
    CCommandTree::tInfoTree* pRoot = NULL;
    if (m_bFinished) {
        pRoot = m_pRoot;
        m_pRoot = NULL;
    }
    return pRoot;
}

bool CCommandTreeBuilder::AppendData(NSCliMsg::InfoPayload* pData)
{
    if (m_bFinished) {
        OUTPUT_ERROR_TRACE("Aleady finished\n");
        return false;
    }
    if (pData->Count == 0) {
        OUTPUT_ERROR_TRACE("No Command Information\n");
        return false;
    }
    if (pData->GetLength() < sizeof(NSCliMsg::InfoPayload)) {
        OUTPUT_ERROR_TRACE("Wrong Information Blocks\n");
        return false;
    }

    size_t idx = 0;
    uint8_t* pCur = pData->Blocks;
    NSCliMsg::Block* pCurBlock = reinterpret_cast<NSCliMsg::Block*>(pData->Blocks);
    if (m_pRoot == NULL) {
        if (pCurBlock->Type != NSCliMsg::BT_ROOT_INFO) {
            OUTPUT_ERROR_TRACE("Response is not about command information\n");
            return false;
        }
        NSCliMsg::RootInfoBlock* pRootBlock =
            reinterpret_cast<NSCliMsg::RootInfoBlock*>(pCurBlock);
        if (pRootBlock->SubCount == 0) {
            // No command supported on server.
            OUTPUT_ERROR_TRACE("Server doesn't support any command\n");
            return false;
        }
        m_pRoot = CreateRoot(pRootBlock->SubCount);
        if (m_pRoot == NULL) {
            OUTPUT_ERROR_TRACE("Create Root failed\n");
            return false;
        }
        m_pCurrent = m_pRoot->GetRoot();
        m_CurrentCount = 0;
        pCur += NSCliMsg::GetBlockSize(pCurBlock);
        pCurBlock = reinterpret_cast<NSCliMsg::Block*>(pCur);
        ++idx;
    }

    ASSERT(m_pCurrent);
    while (idx < pData->Count) {
        CCommandTree::InfoElement* pElem = m_pCurrent->GetElement();
        if (m_CurrentCount == pElem->SubCount) {
            m_pCurrent = GetNextNode(m_pCurrent);
            if (m_pCurrent == NULL) {
                OUTPUT_ERROR_TRACE("Message information is not right\n");
                return false;
            }
            m_CurrentCount = 0;
        }
        CCommandTree::tInfoNode* pNode = CreateNode(m_pCurrent, pCurBlock);
        if (pNode == NULL) {
            OUTPUT_ERROR_TRACE("Create tree node failed\n");
            return false;
        }
        ++m_CurrentCount;
        ++idx;
        pCur += NSCliMsg::GetBlockSize(pCurBlock);
        pCurBlock = reinterpret_cast<NSCliMsg::Block*>(pCur);
    }
    if (idx == pData->Count) {
        m_bFinished = true;
    }
    return true;
}

CCommandTree::tInfoTree* CCommandTreeBuilder::CreateRoot(size_t count)
{
    CCommandTree::tInfoTree* pRoot = new CCommandTree::tInfoTree();
    if (pRoot) {
        CCommandTree::InfoElement* pRootItem =
            reinterpret_cast<CCommandTree::InfoElement*>(
                m_Buffer.Malloc(sizeof(CCommandTree::InfoElement)));
        CCommandTree::tInfoTree::CNode* pRootNode = NULL;
        if (pRootItem) {
            pRootItem->SubCount = count;
            pRootItem->Type = CCommandTree::TYPE_ROOT;
            pRootItem->pItemData = NULL;
            pRootNode = pRoot->CreateRoot(pRootItem);
        }
        // Handle failed case.
        if (pRootNode == NULL) {
            delete pRoot;
            pRoot = NULL;
        }
    }
    return pRoot;
}

CCommandTree::tInfoNode*
CCommandTreeBuilder::CreateNode(
    CCommandTree::tInfoNode* pParent, NSCliMsg::Block* pBlock)
{
    CCommandTree::InfoElement* pElem =
        reinterpret_cast<CCommandTree::InfoElement*>(
            m_Buffer.Malloc(sizeof(CCommandTree::InfoElement)));
    if (pElem == NULL) {
        return NULL;
    }

    bool bRes = false;
    NSCliMsg::CommandInfoBlock* pCmdInfo =
        reinterpret_cast<NSCliMsg::CommandInfoBlock*>(pBlock);
    NSCliMsg::VariableInfoBlock* pVarInfo =
        reinterpret_cast<NSCliMsg::VariableInfoBlock*>(pBlock);
    switch (pBlock->Type) {
    case NSCliMsg::BT_COMMAND_INFO:
        pElem->Type = CCommandTree::TYPE_COMMAND;
        pElem->pItemData =
            m_Buffer.Malloc(sizeof(CCommandTree::CommandItem) + pCmdInfo->NameLength);
        if (pElem->pItemData) {
            CCommandTree::CommandItem* pItem =
                reinterpret_cast<CCommandTree::CommandItem*>(pElem->pItemData);
            pItem->CmdID = pCmdInfo->CommandID;
            memcpy(pItem->Name, pCmdInfo->NameString, pCmdInfo->NameLength);
            pElem->SubCount = pCmdInfo->SubCount;
            bRes = true;
        }
        break;
    case NSCliMsg::BT_VARIABLE_INFO:
        pElem->Type = CCommandTree::TYPE_VARIABLE;
        pElem->pItemData =
            m_Buffer.Malloc(sizeof(CCommandTree::VariableItem) + pVarInfo->NameLength);
        if (pElem->pItemData) {
            CCommandTree::VariableItem* pItem =
                reinterpret_cast<CCommandTree::VariableItem*>(pElem->pItemData);
            pItem->bCharString = pVarInfo->IsCharString();
            pItem->bMandatory = pVarInfo->IsMandatory();
            memcpy(pItem->Name, pVarInfo->NameString, pVarInfo->NameLength);
            pElem->SubCount = pVarInfo->SubCount;
            bRes = true; 
        }
        break;
    default:
        break;
    }

    CCommandTree::tInfoTree::CNode* pNode = NULL;
    if (bRes) {
        pNode = m_pRoot->AddChild(pParent, pElem);
    }
    return pNode;
}

CCommandTree::tInfoNode*
CCommandTreeBuilder::GetNextNode(CCommandTree::tInfoNode* pCur)
{
    CCommandTree::tInfoTree::CDFSTraverser traverse(pCur);
    CCommandTree::tInfoNode* pNode = traverse.GetNext();
    while (pNode) {
        CCommandTree::InfoElement* pElem = pNode->GetElement();
        if (pElem->SubCount > 0) {
            break;
        }
        pNode = traverse.GetNext();
    }
    return pNode;
}


///////////////////////////////////////////////////////////////////////////////
//
// CCommandTreeSerializer Implementation
//
///////////////////////////////////////////////////////////////////////////////
CCommandTreeSerializer::CCommandTreeSerializer(
    const CCommandTree::tInfoNode& cmdRoot) :
    m_pBuffer(NULL),
    m_CmdRoot(cmdRoot),
    m_pCurrent(const_cast<CCommandTree::tInfoNode*>(&cmdRoot))
{
}

CCommandTreeSerializer::~CCommandTreeSerializer()
{
    if (m_pBuffer) {
        free(m_pBuffer);
    }
}

NSCliMsg::InfoPayload* CCommandTreeSerializer::Serialize()
{
    static const size_t bufferSize =
        NSCliMsg::MAX_PACKET_LENGTH - sizeof(NSCliMsg::Message);

    if (m_pCurrent == NULL) {
        return NULL;
    }
    if (m_pBuffer == NULL) {
        m_pBuffer = reinterpret_cast<uint8_t*>(malloc(bufferSize));
        if (m_pBuffer == NULL) {
            OUTPUT_ERROR_TRACE("Create buffer failed\n");
            return NULL;
        }
    }

    uint8_t* pEnd = m_pBuffer + bufferSize;
    uint8_t* pCur = m_pBuffer + sizeof(NSCliMsg::InfoPayload);
    NSCliMsg::InfoPayload* pInfoPayload = NULL;
    size_t count = 0;
    size_t dataLength = 0;
    CCommandTree::tInfoNode* pNode = m_pCurrent;
    CCommandTree::tInfoTree::CDFSTraverser traverse(m_pCurrent);

    do {
        CCommandTree::InfoElement* pElem = pNode->GetElement();
        CCommandTree::CommandItem* pCmdItem =
                reinterpret_cast<CCommandTree::CommandItem*>(pElem->pItemData);
        CCommandTree::VariableItem* pVarItem =
            reinterpret_cast<CCommandTree::VariableItem*>(pElem->pItemData);

        size_t nameLen = 0;
        size_t dataLen = 0;
        void* pData = NULL;
        switch (pElem->Type) {
        case CCommandTree::TYPE_COMMAND:
            ASSERT(pCmdItem);
            nameLen = strlen(pCmdItem->Name) + 1;
            dataLen = nameLen + sizeof(NSCliMsg::CommandInfoBlock);
            if (pCur + dataLen <= pEnd) {
                pData = new (pCur) NSCliMsg::CommandInfoBlock(
                    pCmdItem->CmdID, pElem->SubCount, nameLen, pCmdItem->Name);
            }
            break;
        case CCommandTree::TYPE_VARIABLE:
            ASSERT(pVarItem);
            nameLen = strlen(pVarItem->Name) + 1;
            dataLen = nameLen + sizeof(NSCliMsg::VariableInfoBlock);
            if (pCur + dataLen <= pEnd) {
                pData = new (pCur) NSCliMsg::VariableInfoBlock(
                    pElem->SubCount,
                    pVarItem->bMandatory,
                    pVarItem->bCharString,
                    nameLen, pVarItem->Name);
            }
            break;
        case CCommandTree::TYPE_ROOT:
            ASSERT(pElem->SubCount > 0);
            ASSERT(pElem->pItemData == NULL);
            dataLen = sizeof(NSCliMsg::RootInfoBlock);
            if (pCur + dataLen <= pEnd) {
                pData = new (pCur) NSCliMsg::RootInfoBlock(pElem->SubCount);
            }
            break;
        default:
            ASSERT(false);
            break;
        }
        if (pData == NULL) {
            break;
        }
        dataLength += dataLen;
        pCur += dataLen;
        ++count;
        m_pCurrent = pNode = traverse.GetNext();
    } while (pNode);

    if (count > 0) {
        pInfoPayload = new (m_pBuffer)
            NSCliMsg::InfoPayload(dataLength, count);
    }
    return pInfoPayload;
}
