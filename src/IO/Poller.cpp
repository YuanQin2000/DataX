/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Poller.h"
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <utility>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "PollClient.h"
#include "IOHelper.h"
#include "Common/ErrorNo.h"
#include "Thread/Looper.h"
#include "Tracker/Trace.h"

using std::malloc;
using std::free;
using std::memset;
using std::pair;

CPoller::CPoller(IExtCmdHandler* pHandler) :
    m_hCmdIO{INVALID_IO_HANDLE, INVALID_IO_HANDLE},
    m_hPollIO(INVALID_IO_HANDLE),
    m_PollCount(0),
    m_ReservedPollEventCount(0),
    m_pPollEvents(NULL),
    m_IOClients(),
    m_pExtMsgHandle(pHandler),
    m_CmdReadMemPool(CForwardList::ListNodeSize(sizeof(ITMessage))),
    m_CmdReadCache(&m_CmdReadMemPool),
    m_CmdReadBuffer{0}
{
}

CPoller::~CPoller()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    Exit();
    ClosePollHandles();
    free(m_pPollEvents);
}

CPoller* CPoller::CreateInstance(IExtCmdHandler* pHandler)
{
    CPoller* pInstance = new CPoller(pHandler);
    if (pInstance == NULL) {
        return NULL;
    }

    size_t memorySize = sizeof(struct epoll_event) * EPOLL_EVENT_COUNT_MIN;
    pInstance->m_pPollEvents = reinterpret_cast<struct epoll_event*>(malloc(memorySize));
    if (pInstance->m_pPollEvents == NULL) {
        OUTPUT_ERROR_TRACE("malloc memory failed\n");
        delete pInstance;
        return NULL;
    }
    memset(pInstance->m_pPollEvents, 0, memorySize);
    pInstance->m_ReservedPollEventCount = EPOLL_EVENT_COUNT_MIN;
    if (!pInstance->OpenPollHandles()) {
        delete pInstance;
        return NULL;
    }
    return pInstance;
}

void CPoller::HandleMessage(ITMessage* pMsg)
{
    PollClientData* pData = NULL;
    AsynTaskData* pTask = NULL;

    switch (pMsg->MsgID) {
    case CID_ADD_CLIENT:
        pData = reinterpret_cast<PollClientData*>(pMsg->GetData());
        DoAddClient(pData->pClient, pData->bOwned, true);
        break;
    case CID_REMOVE_CLIENT:
        pData = reinterpret_cast<PollClientData*>(pMsg->GetData());
        DoRemoveClient(pData->pClient, true);
        break;
    case CID_ASYN_TASK:
        pTask = reinterpret_cast<AsynTaskData*>(pMsg->GetData());
        pTask->Task(pTask->pData);
        break;
    case CID_EXT_CMD:
        if (m_pExtMsgHandle) {
            m_pExtMsgHandle->OnMessage(pMsg->GetData());
            break;
        }
        // fall through
    default:
        ASSERT(false, "Discard unknown message: %d\n", pMsg->MsgID);
        break;
    }
}

bool CPoller::ReadMessage(ITMessage* pOutMsg, int timeout /* = -1 */)
{
    // Read from cache first.
    ITMessage* pCacheMsg = reinterpret_cast<ITMessage*>(m_CmdReadCache.First());
    if (pCacheMsg) {
        memcpy(pOutMsg, pCacheMsg, sizeof(ITMessage));
        m_CmdReadCache.PopFront();
        return true;
    }

    bool bRes = false;
    int eventCount = epoll_wait(
        m_hPollIO, m_pPollEvents, m_PollCount, timeout);
    if (eventCount > 0) {
        for (int i = 0; i < eventCount; i++) {
            if (m_pPollEvents[i].data.fd == m_hCmdIO[0]) {
                bRes = GetControlMessages(m_pPollEvents[i].events, pOutMsg);
            } else {
                HandleDataEvent(m_pPollEvents[i].data.fd, m_pPollEvents[i].events);
            }
        }
    } else if (eventCount < 0) {
        if (errno == EINTR) {
            OUTPUT_NOTICE_TRACE("epoll_wait exit: %s\n", strerror(errno));
        } else {
            ASSERT(m_pContext);
            m_pContext->Exit();
            OUTPUT_ERROR_TRACE("epoll_wait failed: %s\n", strerror(errno));
        }
    }
    return bRes;
}

bool CPoller::WriteMessage(ITMessage* pMsg)
{
    bool bRes = false;
    size_t len = NSIOHelper::Write(m_hCmdIO[1], pMsg, sizeof(ITMessage));
    if (len > 0) {
        /**
         * @note The POSIX guarantee the write on the pipe are data integrity:
         * all writen or nothing, if the bytes to write is less than the PIPE_BUF,
         * So we don't need handle the incomplete write, and actually we can't handle,
         * in which case, we need to save the incomplete part, and write it next time
         * once the IO is ready. Unfortunately, this function is invoked by different
         * threads, therefore, access to the incomplete relevant data become the
         * critical section which needs to be locked.
         */
        // ASSERT(len == sizeof(ITMessage));
        bRes = true;
    } else {
        OUTPUT_ERROR_TRACE("Write message failed: %s\n", strerror(ERROR_CODE));
    }
    return bRes;
}

bool CPoller::AddClient(
    CPollClient* pObject, bool bTransferOwnership /* = false */)
{
    ASSERT(pObject);

    if (m_pContext->IsInLoop()) {
        return DoAddClient(pObject, bTransferOwnership, true);
    }

    bool bRes = false;
    PollClientData data(pObject, bTransferOwnership);
    ITMessage msg(CID_ADD_CLIENT);
    if (msg.SetData(&data, sizeof(data))) {
        bRes = WriteMessage(&msg);
    }
    return bRes;
}

bool CPoller::RemoveClient(CPollClient* pObject)
{
    ASSERT(pObject);

    if (m_pContext->IsInLoop()) {
        DoRemoveClient(pObject, true);
        return true;
    }

    ITMessage msg(CID_REMOVE_CLIENT);
    msg.SetExtData(pObject);
    return WriteMessage(&msg);
}

bool CPoller::DoAddClient(
    CPollClient* pObject,
    bool bTransferOwnership /* = false */,
    bool bNotify /* = false */)
{
    tIOHandle hIO = pObject->PollIO();
    uint32_t event = pObject->PollEventMask();

    ASSERT(hIO != INVALID_IO_HANDLE);
    ASSERT(event != 0);

#ifdef __DEBUG__
    VerifyClient(pObject, false);
#endif

    bool bRes = false;
    pair<map<tIOHandle, PollClientData>::iterator, bool> res =
        m_IOClients.insert(pair<tIOHandle, PollClientData>(
                hIO, PollClientData(pObject, bTransferOwnership)));
    if (res.second) {
        if (AddPollIO(hIO, event)) {
            bRes = true;
        } else {
            m_IOClients.erase(res.first);
        }
    }
    if (bNotify) {
        pObject->OnAttached(bRes);
    }
    return bRes;
}

void CPoller::DoRemoveClient(CPollClient* pObject, bool bNotify /* = false */)
{
    tIOHandle hIO = pObject->PollIO();
    ASSERT(hIO != INVALID_IO_HANDLE);

#ifdef __DEBUG__
    VerifyClient(pObject, true);
#endif

    map<tIOHandle, PollClientData>::const_iterator iter = m_IOClients.find(hIO);
    if (iter != m_IOClients.end()) {
        ASSERT(!iter->second.bOwned);
        // If the Poller owned the client,
        // then the client should not be removed from outside components.
        m_IOClients.erase(iter);
    }
    RemovePollIO(hIO);
    if (bNotify) {
        pObject->OnDetached();
    }
}

bool CPoller::OpenPollHandles()
{
    if (pipe(m_hCmdIO) != 0) {
        OUTPUT_ERROR_TRACE("pipe: %s\n", strerror(errno));
        return false;
    }
    size_t pipeBufferSize = fcntl(m_hCmdIO[0], F_GETPIPE_SZ);
    if (pipeBufferSize < sizeof(ITMessage)) {
        // Make sure the Pipe buffer is larger than message to write, just in case.
        OUTPUT_DEBUG_TRACE(
            "Pipe buffer size: %d, ITMessage size: %d\n",
            pipeBufferSize, sizeof(ITMessage));
        int newPipeBufferSize = (sizeof(ITMessage) << 4);
        if (fcntl(m_hCmdIO[0], F_SETPIPE_SZ, newPipeBufferSize) != newPipeBufferSize) {
            OUTPUT_ERROR_TRACE("Expand pipe buffer size failed. %s\n", strerror(errno));
            ClosePollHandles();
            return false;
        }
        OUTPUT_DEBUG_TRACE("Expand pipe buffer size to %d\n", newPipeBufferSize);
    }

    if (!NSIOHelper::SetIOBlockMode(m_hCmdIO[0], false) ||
        !NSIOHelper::SetIOBlockMode(m_hCmdIO[1], false)) {
        OUTPUT_ERROR_TRACE("SetIOBlockMode failed: %s\n", strerror(errno));
        ClosePollHandles();
        return false;
    }
    m_hPollIO = epoll_create1(0);
    if (m_hPollIO < 0) {
        OUTPUT_ERROR_TRACE("epoll_create1: %s\n", strerror(errno));
        ClosePollHandles();
        return false;
    }
    if (!AddPollIO(m_hCmdIO[0], EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP)) {
        ClosePollHandles();
        return false;
    }
    return true;
}

void CPoller::ClosePollHandles()
{
    if (m_hCmdIO[0] != INVALID_IO_HANDLE) {
        close(m_hCmdIO[0]);
        m_hCmdIO[0] = INVALID_IO_HANDLE;
    }
    if (m_hCmdIO[1] != INVALID_IO_HANDLE) {
        close(m_hCmdIO[1]);
        m_hCmdIO[1] = INVALID_IO_HANDLE;
    }
    if (m_hPollIO != INVALID_IO_HANDLE) {
        close(m_hPollIO);
        m_hPollIO = INVALID_IO_HANDLE;
    }
}

bool CPoller::AddPollIO(tIOHandle hIO, uint32_t eventMask)
{
    ASSERT(hIO != INVALID_IO_HANDLE);
    ASSERT(!NSIOHelper::IsIOBlocked(hIO));
    ASSERT(eventMask != 0);

    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = hIO;
    event.events = eventMask |
        EPOLLERR |
        EPOLLET |
        EPOLLHUP |
        EPOLLRDHUP |
        EPOLLPRI;
    if (m_PollCount >= m_ReservedPollEventCount) {
        void* pNew = realloc(
            m_pPollEvents,
            sizeof(struct epoll_event) * m_ReservedPollEventCount + EPOLL_EVENT_COUNT_MIN);
        if (pNew) {
            m_pPollEvents = reinterpret_cast<struct epoll_event*>(pNew);
            m_ReservedPollEventCount += EPOLL_EVENT_COUNT_MIN;
        } else {
            OUTPUT_ERROR_TRACE("realloc failed\n");
            return false;
        }
    }
    if (epoll_ctl(m_hPollIO, EPOLL_CTL_ADD, hIO, &event) < 0) {
        OUTPUT_ERROR_TRACE("epoll_ctl: %s", strerror(errno));
        return false;
    }
    ++m_PollCount;
    return true;
}

void CPoller::RemovePollIO(tIOHandle hIO)
{
    ASSERT(hIO != INVALID_IO_HANDLE);

    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = hIO;
    if (epoll_ctl(m_hPollIO, EPOLL_CTL_DEL, hIO, &event) == 0) {
        ASSERT(m_PollCount > 0);
        --m_PollCount;
    } else {
        OUTPUT_ERROR_TRACE("epoll_ctl: %s", strerror(errno));
        ASSERT(false);
    }
}

void CPoller::Exit()
{
    map<tIOHandle, PollClientData>::const_iterator iter = m_IOClients.begin();
    map<tIOHandle, PollClientData>::const_iterator iterEnd = m_IOClients.end();
    while (iter != iterEnd) {
        CPollClient* pClient = iter->second.pClient;
        if (iter->second.bOwned) {
            delete pClient;
        }
        ++iter;
    }
    m_IOClients.clear();
}

bool CPoller::GetControlMessages(uint32_t events, ITMessage* pOutMsg)
{
    bool bRes = false;
    if (events & EPOLLIN) {
        size_t readLen = NSIOHelper::Read(
            m_hCmdIO[0], m_CmdReadBuffer, sizeof(m_CmdReadBuffer));
        if (readLen > 0) {
            memcpy(pOutMsg, m_CmdReadBuffer, sizeof(ITMessage));
            bRes = true;
        }

        // Read out all remain messages and cache them.
        ITMessage* pMsgCur = &m_CmdReadBuffer[1];
        ITMessage* pMsgEnd = reinterpret_cast<ITMessage*>(
            reinterpret_cast<uint8_t*>(m_CmdReadBuffer) + readLen);
        do {
            while (pMsgCur < pMsgEnd) {
                if (!m_CmdReadCache.PushBack(pMsgCur)) {
                    OUTPUT_ERROR_TRACE("Push back message failed.\n");
                }
                ++pMsgCur;
            }
            if (readLen < sizeof(m_CmdReadBuffer)) {
                break;
            }
            readLen = NSIOHelper::Read(
                m_hCmdIO[0], m_CmdReadBuffer, sizeof(m_CmdReadBuffer));
            if (readLen == 0) {
                break;
            }
            pMsgCur = m_CmdReadBuffer;
            pMsgEnd = reinterpret_cast<ITMessage*>(
                reinterpret_cast<uint8_t*>(m_CmdReadBuffer) + readLen);
        } while (true);
    }

    if ((events & EPOLLHUP) || (events & EPOLLRDHUP)) {
        // Control command IO is closed.
        m_pContext->Exit();
    }
    return bRes;
}

void CPoller::HandleDataEvent(tIOHandle hIO, uint32_t events)
{
    CPollClient* pClient = NULL;
    bool bOwned = false;
    map<tIOHandle, PollClientData>::const_iterator iter = m_IOClients.find(hIO);
    if (iter != m_IOClients.end()) {
        pClient = iter->second.pClient;
        bOwned = iter->second.bOwned;
    }

    ASSERT(pClient);

    if ((events & EPOLLIN) || (events & EPOLLPRI)) {
        pClient->OnIncomingData();
    }
    if (events & EPOLLOUT) {
        pClient->OnOutgoingReady();
    }
    if ((events & EPOLLHUP) || (events & EPOLLRDHUP)) {
        // On peer closed
        pClient->OnPeerClosed();
        if (bOwned) {
            m_IOClients.erase(iter);
            delete pClient;
        }
    }
    if (events & EPOLLERR) {
        OUTPUT_ERROR_TRACE("Poll Error on IO: %d\n", hIO);
    }
}


#ifdef __DEBUG__
void CPoller::VerifyClient(CPollClient* pObject, bool bExist)
{
    bool bFound = false;
    map<tIOHandle, PollClientData>::const_iterator iter = m_IOClients.begin();
    map<tIOHandle, PollClientData>::const_iterator iterEnd = m_IOClients.end();

    while (iter != iterEnd) {
        if (iter->second.pClient == pObject) {
            bFound = true;
            break;
        }
        ++iter;
    }
    ASSERT(bFound == bExist, "expected %s found\n", bExist ? "exist but NOT" : "NOT exist but");
}
#endif
