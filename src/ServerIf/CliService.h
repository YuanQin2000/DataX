/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLI_SERVICE_H__
#define __CLI_SERVICE_H__

#include <set>
#include "Common/Typedefs.h"
#include "Common/Singleton.h"
#include "ServerIO.h"
#include "Thread/Lock.h"
#include "Thread/Condition.h"
#include "IO/Poller.h"

using std::set;

class CUDSStreamClient;
class CCliHandler;
class CLooper;
class CCliService :
    public CPoller::IExtCmdHandler,
    public CServerIO::IServiceHandle,
    public CSingleton<CCliService>
{
public:
    enum MessageTargetID {
        MSGID_CLI_HANDLER = 0,
        MSGID_COUNT,
    };

    struct CliCommand {
        uint8_t CmdID;
        uint8_t DataID;
        void* pData;
        CCliHandler* pHandler;
    };

    // From CPoller::IExtCmdHandler
    void OnMessage(void* pMsg);

    // From CServerIO::IServiceHandle
    void OnClientConnected(CIOContext* pClient);

    bool Start();
    void Stop();
    void WaitStopped();

    bool WriteMessage(CliCommand* pCliCmd)
    {
        return m_pPoller->SendExtCommand(pCliCmd, sizeof(CliCommand));
    }

private:
    CCliService();
    ~CCliService();

private:
    bool m_bRunning;
    CPoller* m_pPoller; // Owned
    CLooper* m_pLoop; // Owned
    set<CCliHandler*> m_CliHandlers; // Owned
    CCriticalSection m_CS;
    CCondition m_CondVar;

    static const int MAX_NUM_CLIENTS = 10;
    friend class CSingleton<CCliService>;
};

#endif