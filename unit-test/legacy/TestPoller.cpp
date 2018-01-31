/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>

#include "Typedefs.h"
#include "Tracker/Debug.h"
#include "IO/Poller.h"
#include "IO/PollClient.h"
#include "Thread/MessageHandle.h"

using std::fprintf;
using std::printf;

class CTCPDataPrinter : public IPollClient,
                        public ITimerHandle
{
public:
    CTCPDataPrinter(tIOHandle hIO);
    ~CTCPDataPrinter();

public:
    void OnTimeout(tTimerID timerID);
    void OnTimeCreated(int sessionID, tTimerID timerID);
    void OnTimerDeleted(tTimerID timerID);

public:
    tIOHandle PollIO() const;
    uint32_t  PollEventMask() const;
    void OnIncomingData();
    void OnOutgoingData();
    void OnPeerClosed();
    void OnControlEvent(CPoller::ControlEvent event,
                        Argument arg1 = Argument(),
                        Argument arg2 = Argument());

private:
    void PrintRawByte(char* pData, size_t len);

private:
    tIOHandle m_hIO;
    char      m_Buffer[4096];
    CPoller*  m_pPoller;
    int       m_TimerID;
};

CTCPDataPrinter::CTCPDataPrinter(tIOHandle hIO) : m_hIO(hIO)
{
    ASSERT(hIO >= 0);
    m_pPoller = NULL;
    m_TimerID = -1;
    printf("CTCPDataPrinter::CTCPDataPrinter: %d\n", hIO);
}

CTCPDataPrinter::~CTCPDataPrinter()
{
    printf("CTCPDataPrinter::~CTCPDataPrinter: %d\n", m_hIO);
    close(m_hIO);
}

void CTCPDataPrinter::OnTimeout(tTimerID timerID)
{
    struct timeval current;
    gettimeofday(&current, NULL);
    printf("[%ld:%ld] Timer %d is TIME OUT\n", current.tv_sec, current.tv_usec, timerID);
}

void CTCPDataPrinter::OnTimeCreated(int sessionID, tTimerID timerID)
{
    struct timeval current;
    gettimeofday(&current, NULL);
    printf("[%ld:%ld] Session: %d, Timer %d is Created\n", current.tv_sec, current.tv_usec, sessionID, timerID);
    m_TimerID = timerID;
}

void CTCPDataPrinter::OnTimerDeleted(tTimerID timerID)
{
    printf("Timer %d is Deleted\n", timerID);
    m_TimerID = -1;
}

tIOHandle CTCPDataPrinter::PollIO() const
{
    return m_hIO;
}

uint32_t CTCPDataPrinter::PollEventMask() const
{
    return EPOLLIN;
}

void CTCPDataPrinter::OnIncomingData()
{
    ASSERT(m_pPoller);

    printf("CTCPDataPrinter: OnIncomingData\n");

    int len = read(m_hIO, m_Buffer, sizeof(m_Buffer) - 1);
    if (len > 0) {
        m_Buffer[len] = '\0';
        printf("Receive data: %s\n", m_Buffer);
        PrintRawByte(m_Buffer, len);
        if (m_Buffer[0] == 'T') {
            int milliseconds = atoi(&m_Buffer[1]);
            if (milliseconds > 0) {
                struct timeval current;
                gettimeofday(&current, NULL);
                m_TimerID = m_pPoller->StartTimer(milliseconds, this);
                printf("[%ld:%ld] Timer %d created directly\n", current.tv_sec, current.tv_usec, m_TimerID);
            }
        } else if (m_Buffer[0] == 'C') {
            if (m_TimerID >= 0) {
                printf("Stop timer: %d\n", m_TimerID);
                m_pPoller->StopTimer(m_TimerID);
            }
        } else if (m_Buffer[0] == 'E') {
            printf("Exit\n");
            m_pPoller->Exit();
        }
    } else if (len < 0) {
        fprintf(stderr, "read return: %d: %s\n", len, strerror(errno));
    }
}

void CTCPDataPrinter::OnOutgoingData()
{
    printf("CTCPDataPrinter: OnOutgoingData\n");
}

void CTCPDataPrinter::OnPeerClosed()
{
    printf("CTCPDataPrinter: OnPeerClosed\n");
}

void CTCPDataPrinter::OnControlEvent(CPoller::ControlEvent event, Argument arg1, Argument arg2)
{
    switch (event) {
        case CPoller::EVENT_ATTACHED:
            m_pPoller = reinterpret_cast<CPoller*>(arg1.pValue);
            break;
        case CPoller::EVENT_DETACHED:
            m_pPoller = NULL;
            break;
        case CPoller::EVENT_POLLER_EXIT:
            break;
        case CPoller::EVENT_ERROR:
            break;
        default:
            ASSERT(false);
            break;
    }
}

void CTCPDataPrinter::PrintRawByte(char* pData, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%d ", static_cast<int>(pData[i]));
    }
    printf("\n");
}


class CTCPServer : public IPollClient,
                   public IMessageHandle
{
// From IMessageHandle
public:
    void Receive(Message* pMsg);

public:
    CTCPServer(unsigned short listenedPort, int backlog);
    ~CTCPServer();

public:
    bool Start();
    void Stop();

public:
    tIOHandle PollIO() const;
    uint32_t  PollEventMask() const;
    void OnIncomingData();
    void OnOutgoingData();
    void OnPeerClosed();
    void OnControlEvent(CPoller::ControlEvent event,
                        Argument arg1 = Argument(),
                        Argument arg2 = Argument());

    void Test();

private:
    DISALLOW_COPY_CONSTRUCTOR(CTCPServer);
    DISALLOW_ASSIGN_OPERATOR(CTCPServer);
    DISALLOW_DEFAULT_CONSTRUCTOR(CTCPServer);

private:
    enum CommandID {
        CID_COMMAND1,
        CID_COMMAND2,
        CID_COMMAND3
    };

private:
    CPoller*  m_pPoller;     // Owner
    tIOHandle m_hIO;
    unsigned short m_Port;
    const int m_Backlog;
};


CTCPServer::CTCPServer(unsigned short listenedPort, int backlog)
    : m_pPoller(NULL), m_hIO(-1), m_Port(listenedPort), m_Backlog(backlog)
{
    ASSERT(backlog > 0);
}

CTCPServer::~CTCPServer()
{
    printf("CTCPServer::~CTCPServer\n");
    Stop();
    if (m_pPoller) {
        CPoller::DestroyInstance(m_pPoller);
    }
}

bool CTCPServer::Start()
{
    int sock = -1;
    int optVal = 1;
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(m_Port);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket: %s\n", strerror(errno));
        return false;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0) {
        printf("setsockopt: %s\n", strerror(errno));
        close(sock);
        return false;
    }
    if (bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("bind: %s\n", strerror(errno));
        close(sock);
        return false;
    }
    if (listen(sock, m_Backlog) < 0) {
        printf("listen: %s\n", strerror(errno));
        close(sock);
        return false;
    }
    m_hIO = sock;
    m_pPoller = CPoller::CreateInstance(this, CThread::PRIORITY_NORMAL, NULL);
    if (m_pPoller && m_pPoller->AddPollClient(this) != PS_FAILED) {
        return true;
    }
    return false;
}

void CTCPServer::Stop()
{
    printf("CTCPServer::Stop ==>\n");
    m_pPoller->Exit();
    sleep(1);
    if (m_hIO >= 0) {
        close(m_hIO);
        m_hIO = -1;
    }
}

tIOHandle CTCPServer::PollIO() const
{
    ASSERT(m_hIO);

    return m_hIO;
}

uint32_t CTCPServer::PollEventMask() const
{
    return EPOLLIN;
}

void CTCPServer::OnIncomingData()
{
    printf("CTCPServer: OnIncomingData\n");

    CTCPDataPrinter* pTCPPrinter = NULL;
    struct sockaddr_in clientAddress;
    socklen_t len = sizeof(clientAddress);
    tIOHandle clientIO = accept(m_hIO, (struct sockaddr*)&clientAddress, &len);
    if (clientIO < 0) {
        printf("accept: %s\n", strerror(errno));
        return;
    }
    pTCPPrinter = new CTCPDataPrinter(clientIO);
    if (pTCPPrinter) {
        if (!m_pPoller->AddPollClient(pTCPPrinter, true)) {
            delete pTCPPrinter;
        }
    } else {
        close(clientIO);
    }
}

void CTCPServer::OnOutgoingData()
{
    printf("CTCPServer: OnOutgoingData\n");
}

void CTCPServer::OnPeerClosed()
{
    printf("CTCPServer: OnPeerClosed\n");
}

void CTCPServer::OnControlEvent(CPoller::ControlEvent event,
                                Argument arg1,
                                Argument arg2)
{
    struct timeval current;
    gettimeofday(&current, NULL);
    printf("[%ld:%ld] CTCPServer: OnControlEvent: %d: %d\n", current.tv_sec, current.tv_usec, event, arg1.nValue);
}

void CTCPServer::Receive(Message* pMsg)
{
    ASSERT(pMsg);

    switch (pMsg->MsgID) {
    case CID_COMMAND1:
        printf("Receive => CID_COMMAND1\n");
        break;
    case CID_COMMAND2:
        printf("Receive => CID_COMMAND2\n");
        break;
    case CID_COMMAND3:
        printf("Receive => CID_COMMAND3\n");
        break;
    default:
        ASSERT(false);
        break;
    }
}

void CTCPServer::Test()
{
    Message msg;
    msg.MsgID = CID_COMMAND1;
    m_pPoller->SendMessage(&msg);
    sleep(4);
    msg.MsgID = CID_COMMAND2;
    m_pPoller->SendMessage(&msg);
    sleep(4);
    msg.MsgID = CID_COMMAND3;
    m_pPoller->SendMessage(&msg);
}

int main()
{
    CTCPServer* pTCPServer = new CTCPServer(60204, 10);
    if (pTCPServer) {
        if (pTCPServer->Start()) {
            sleep(4);
            pTCPServer->Test();
            sleep(10000000);
        }
    }
    delete pTCPServer;
    return 0;
}
