/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATA_COM_CONNNECTION_RUNNER_H__
#define __DATA_COM_CONNNECTION_RUNNER_H__

#include "Common/Typedefs.h"
#include "Common/Pointer.h"
#include "IO/Poller.h"
#include "Connection.h"
#include <utility>
#include <map>
#include <sys/socket.h>

using std::pair;
using std::map;

class CLooper;
class CRequest;
class CConnectionRunner : public CPoller::IExtCmdHandler
{
public:
    ~CConnectionRunner();

    // From CPoller::IExtCmdHandler
    void OnMessage(void* pMsg);

    bool AddConnection(CConnection* pConn);
    bool RemoveConnection(CConnection* pConn);

    /**
     * @brief Push data to the target peer specified by socket address.
     * @param pReq The request to be pushed
     * @param pTarget Specify the target address.
     * @note The return result doesn't mean the data has been sent successed.
     */
    bool PushRequest(CRequest* pReq, const sockaddr* pTarget);

    static CConnectionRunner* CreateInstance(const char* pName);

private:
    CConnectionRunner();

    bool DoAddConnection(CConnection* pConn)
    {
        CPointer address(pConn->GetPeerAddress(), sizeof(sockaddr));
        pair<map<CPointer, CConnection*>::iterator, bool> res = 
            m_Connections.insert(map<CPointer, CConnection*>::value_type(address, pConn));
        return res.second;
    }

    void DoRemoveConnection(CConnection* pConn);
    bool DoPushRequest(CRequest* pReq, const sockaddr* pTarget);
    CConnection* FindConnection(const sockaddr* pTarget);

private:
    enum CmdID {
        CID_ADD_CONNECTION = 0,
        CID_REMOVE_CONNECTION,
        CID_PUSH_REQUEST,
    };

    struct Command {
        CmdID ID;
        void* pData;
        sockaddr DataAddress[0];
    };

    map<CPointer, CConnection*> m_Connections;
    CPoller* m_pPoller; // Owned
    CLooper* m_pLoop;   // Owned

    DISALLOW_COPY_CONSTRUCTOR(CConnectionRunner);
    DISALLOW_ASSIGN_OPERATOR(CConnectionRunner);
};

#endif