/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_MSG_HELPER_H__
#define __CLIENT_MSG_HELPER_H__

#include <set>
#include <vector>
#include "Common/Typedefs.h"
#include "Common/Singleton.h"
#include "ClientIf/CliMsg.h"
#include "ClientIf/CommandTree.h"

using std::set;
using std::vector;


class IResultHandler
{
public:
    virtual ~IResultHandler() {}
    virtual void OnResult(
        uint16_t sessionID,
        NSCliMsg::MsgStatusCode sc,
        uint8_t* pData = NULL,
        size_t len = 0,
        bool bLast = true) = 0;
};

class ICliCommandHelper
{
public:
    virtual ~ICliCommandHelper() {}

    virtual const char* GetCommandName() const = 0;
    virtual CCommandTree::tInfoNode* GetHint() = 0;
    virtual void ExecuteCommand(
        uint16_t sessionID,
        CVector& param,
        IResultHandler& resultHandler) = 0;
};

class CStringIndex;
class CCliCmdHelperManager : public CSingleton<CCliCmdHelperManager>
{
public:
    typedef vector<NSCliMsg::InfoPayload*> tHintMsgVector;

    const tHintMsgVector& GetHint();
    void ExecuteCommand(
        uint16_t sessionID,
        CVector& command,
        IResultHandler& resultHandler);

    bool RegisterCliMsgHelper(ICliCommandHelper* pHelper);

protected:
    CCliCmdHelperManager();
    ~CCliCmdHelperManager();

private:
    void ReleaseInfoMsg(tHintMsgVector& msgVector);

    typedef struct {
        uint8_t Index;
        ICliCommandHelper* pHelper;
    } tCliCommandHelper;

    tCliCommandHelper m_CliMsgHelpers[256];
    set<const char*, tStringCompareFunc> m_CliHelperNames;

    CCommandTree::tInfoNode m_Root;
    CCommandTree::InfoElement m_RootElem;
    tHintMsgVector m_CommandInfoMsg;
    bool m_bCreateMsg;

    friend class CSingleton<CCliCmdHelperManager>;
};

class CCliCmdHelperRegister
{
public:
    CCliCmdHelperRegister(ICliCommandHelper* pHelper);
    ~CCliCmdHelperRegister();

private:
    ICliCommandHelper* m_pHelper;   // Owned
};

#endif