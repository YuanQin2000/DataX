/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_IF_COMMAND_TREE_H__
#define __CLIENT_IF_COMMAND_TREE_H__

#include "Common/Typedefs.h"
#include "CliMsg.h"
#include "Common/Tree.h"
#include "Memory/LazyBuffer.h"

#ifdef __DEBUG__
#include <stdio.h>
#endif

class CIOContext;
class CCommandTree
{
public:
    enum InfoType {
        TYPE_ROOT,
        TYPE_COMMAND,
        TYPE_VARIABLE,
        TYPE_COUNT
    };

    struct InfoElement {
        uint8_t Type;
        uint8_t SubCount;
        void* pItemData;

        InfoElement(uint8_t type, uint8_t count, void* pData) :
            Type(type), SubCount(count), pItemData(pData) {}

        const char* GetName()
        {
            if (Type == TYPE_COMMAND) {
                return reinterpret_cast<CommandItem*>(pItemData)->Name;
            }
            if (Type == TYPE_VARIABLE) {
                return reinterpret_cast<VariableItem*>(pItemData)->Name;
            }
            return NULL;
        }

        bool operator<(const InfoElement& rhs) const
        {
            ASSERT(false);
            return false;
        }

#ifdef __DEBUG__
        void Dump();
#endif

    };

    struct VariableItem {
        bool bMandatory;
        bool bCharString;
        char Name[0];
    };

    struct CommandItem {
        uint8_t CmdID;
        char Name[0];
    };

    typedef CTree<InfoElement> tInfoTree;
    typedef CTree<InfoElement>::CNode tInfoNode;

    ~CCommandTree();

    tInfoNode* GetRoot() const { return m_pInfoTree->GetRoot(); }
    tInfoNode* FindCommandNode(tInfoNode* pNode, const char* pString);
    tInfoNode* FindVariableNode(tInfoNode* pNode, const char* pString = NULL);
    tInfoNode* FindFitNode(tInfoNode* pNode, const char* pString);

    static CCommandTree* CreateInstance(CIOContext& io);

protected:
    CCommandTree(CIOContext& io);

private:
    tInfoTree* CreateInfoTree();
    bool HandleMessage(NSCliMsg::Message* pMsg, tInfoTree::CNode* pContext);
    bool AddInfoNode(NSCliMsg::Block* pInfo, size_t blockSize, tInfoTree::CNode* pContext);
    tInfoTree::CNode* GetNextNode(tInfoTree::CNode* pCur);

private:
    CIOContext& m_IO;
    tInfoTree* m_pInfoTree;   // Owned
    CLazyBuffer m_Buffer;

    DISALLOW_COPY_CONSTRUCTOR(CCommandTree);
    DISALLOW_ASSIGN_OPERATOR(CCommandTree);
    DISALLOW_DEFAULT_CONSTRUCTOR(CCommandTree);
};


class CCommandTreeBuilder
{
public:
    CCommandTreeBuilder(CLazyBuffer& buffer);
    ~CCommandTreeBuilder();

    CCommandTree::tInfoTree* CreateInfoTree();
    bool AppendData(NSCliMsg::InfoPayload* pData);
    bool IsFinished() const { return m_bFinished; }

private:
    CCommandTree::tInfoTree* CreateRoot(size_t count);
    CCommandTree::tInfoNode* CreateNode(
        CCommandTree::tInfoNode* pParent, NSCliMsg::Block* pBlock);
    CCommandTree::tInfoNode* GetNextNode(CCommandTree::tInfoNode* pCur);

private:
    CCommandTree::tInfoTree* m_pRoot;
    CCommandTree::tInfoNode* m_pCurrent;
    CLazyBuffer& m_Buffer;
    bool m_bFinished;
    uint8_t m_CurrentCount;

    DISALLOW_COPY_CONSTRUCTOR(CCommandTreeBuilder);
    DISALLOW_ASSIGN_OPERATOR(CCommandTreeBuilder);
    DISALLOW_DEFAULT_CONSTRUCTOR(CCommandTreeBuilder);
};


class CCommandTreeSerializer
{
public:
    CCommandTreeSerializer(const CCommandTree::tInfoNode& cmdRoot);
    ~CCommandTreeSerializer();

    NSCliMsg::InfoPayload* Serialize();

private:
    uint8_t* m_pBuffer;
    const CCommandTree::tInfoNode& m_CmdRoot;
    CCommandTree::tInfoNode* m_pCurrent;

    DISALLOW_COPY_CONSTRUCTOR(CCommandTreeSerializer);
    DISALLOW_ASSIGN_OPERATOR(CCommandTreeSerializer);
    DISALLOW_DEFAULT_CONSTRUCTOR(CCommandTreeSerializer);
};

#endif
