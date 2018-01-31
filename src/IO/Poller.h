/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __IO_POLLER_H__
#define __IO_POLLER_H__

#include <map>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include "Common/Typedefs.h"
#include "Memory/MemoryPool.h"
#include "Thread/ITMessage.h"
#include "Thread/Looper.h"

using std::map;

class CPollClient;
class CPoller :
    public IRunner,
    public CMsgSwitch
{
public:
    typedef void (*tAsynTask)(void* pArg);

    class IExtCmdHandler
    {
    public:
        virtual ~IExtCmdHandler() {}
        virtual void OnMessage(void* pMsg) = 0;
    };

    ~CPoller();

    // From IRunner
    void HandleMessage(ITMessage* pMsg);

    // From CMsgSwitch
    bool ReadMessage(ITMessage* pOutMsg, int timeout = -1);
    bool WriteMessage(ITMessage* pMsg);

    bool AddClient(CPollClient* pObject, bool bTransferOwnership = false);
    bool RemoveClient(CPollClient* pObject);

    bool PostAsynTask(tAsynTask task, void* pData)
    {
        ITMessage msg(CID_ASYN_TASK);
        AsynTaskData asynData(task, pData);
        if (msg.SetData(&asynData, sizeof(AsynTaskData))) {
            return WriteMessage(&msg);
        }
        return false;
    }

    bool SendExtCommand(void* pExtCmd, size_t len)
    {
        ITMessage msg(CID_EXT_CMD);
        if (msg.SetData(pExtCmd, len)) {
            return WriteMessage(&msg);
        }
        return false;
    }

    static CPoller* CreateInstance(IExtCmdHandler* pHandler);

private:
    CPoller(IExtCmdHandler* pHandler);

    bool OpenPollHandles();
    void ClosePollHandles();
    void Exit();

    bool DoAddClient(
        CPollClient* pObject, bool bTransferOwnership = false, bool bNotify = false);
    void DoRemoveClient(CPollClient* pObject, bool bNotify = false);

    bool AddPollIO(tIOHandle hIO, uint32_t eventMask);
    void RemovePollIO(tIOHandle hIO);

    bool GetControlMessages(uint32_t events, ITMessage* pOutMsg);
    void HandleDataEvent(tIOHandle hIO, uint32_t events);

#ifdef __DEBUG__
    void VerifyClient(CPollClient* pObject, bool bExist);
#endif

private:
    enum PollMsgID {
        CID_ADD_CLIENT = USER_MSGID_BEGIN,
        CID_REMOVE_CLIENT,
        CID_EXT_CMD,
        CID_ASYN_TASK,
        CID_MAX
    };

    struct PollClientData {
        CPollClient* pClient;
        bool bOwned;

        PollClientData(CPollClient* client, bool owned) : pClient(client), bOwned(owned) {}
    };

    struct AsynTaskData {
        tAsynTask Task;
        void* pData;

        AsynTaskData(tAsynTask task, void* data) : Task(task), pData(data) {}
    };

    tIOHandle m_hCmdIO[2];
    tIOHandle m_hPollIO;
    size_t m_PollCount;
    size_t m_ReservedPollEventCount;
    struct epoll_event* m_pPollEvents;
    map<tIOHandle, PollClientData> m_IOClients;
    IExtCmdHandler* m_pExtMsgHandle;    // Not owned
    CMemoryPool m_CmdReadMemPool;
    CForwardList m_CmdReadCache;
    ITMessage m_CmdReadBuffer[256];

    static const size_t EPOLL_EVENT_COUNT_MIN = 32;

    DISALLOW_COPY_CONSTRUCTOR(CPoller);
    DISALLOW_ASSIGN_OPERATOR(CPoller);
    DISALLOW_DEFAULT_CONSTRUCTOR(CPoller);
};

#endif