/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Connection.h"
#include "Controller.h"
#include "Configure.h"
#include "ConnectionRunner.h"
#include "Common/OctetBuffer.h"
#include "IO/SSLClient.h"
#include "Tracker/Trace.h"

CConnection::CConnection(
    CConnectionRunner& runContext,
    CPoller& poller,
    CIOContext* pIO,
    CController* pController,
    const sockaddr* pAddr) :
    CPollClient(pIO->GetHandle(), EPOLLIN | EPOLLOUT),
    m_MemPool(CDuplexList::ListNodeSize()),
    m_PendingRequests(&m_MemPool),
    m_WaitingRequests(&m_MemPool),
    m_pSendingRequest(NULL),
    m_pIO(pIO),
    m_pController(pController),
    m_pIdleController(NULL),
    m_RunContext(runContext),
    m_Poller(poller),
    m_Error(EC_SUCCESS),
    m_PeerAddress(),
    m_pInBuffer(NULL),
    m_pOutBuffer(NULL)
{
    ASSERT(pIO);
    ASSERT(pAddr);

    memcpy(&m_PeerAddress, pAddr, sizeof(m_PeerAddress));
}

CConnection::~CConnection()
{
    ASSERT_IF(m_pController != NULL, m_pIdleController == NULL);
    ASSERT_IF(m_pIdleController != NULL, m_pController == NULL);

    m_Poller.RemoveClient(this);

    delete m_pIO;
    delete m_pController;
    delete m_pIdleController;
    delete m_pOutBuffer;
    delete m_pInBuffer;
}

CConnection* CConnection::CreateInstance(
    CConnectionRunner& runContext,
    CPoller& poller,
    CRequest* pRequest,
    const sockaddr* pAddr)
{
    CController* pController = NULL;
    CConnection* pInstance = NULL;
    CIOContext* pIO = pRequest->CreateIOContext(pAddr);
    CConfigure& configure(pRequest->GetConfigure());
    if (configure.CreateController(&pController, pRequest) && pIO) {
        pInstance = new CConnection(runContext, poller, pIO, pController, pAddr);
    }

    if (pInstance) {
        pIO = NULL;     // pIO ownership has been transferred to connection.
        if (pController) {
            pController->SetConnection(pInstance);
            pController = NULL; // pController ownership has been transferred to connection.
        }

        size_t inSize = configure.InBufferSize();
        size_t outSize = configure.OutBufferSize();
        uint8_t* pInBuf = reinterpret_cast<uint8_t*>(malloc(inSize));
        uint8_t* pOutBuf = reinterpret_cast<uint8_t*>(malloc(outSize));
        if (pInBuf && pOutBuf) {
            pInstance->m_pInBuffer = new COctetBuffer(pInBuf, inSize, ReleaseBuffer);
            pInstance->m_pOutBuffer = new COctetBuffer(pOutBuf, outSize, ReleaseBuffer);
        }
        if (pInstance->m_pInBuffer == NULL ||
            pInstance->m_pOutBuffer == NULL ||
            !poller.AddClient(pInstance)) {
            delete pInstance;
            pInstance = NULL;
        }
    }

    // delete objects if has.
    delete pIO;
    delete pController;

    return pInstance;
}

// TODO
void CConnection::OnAttached(bool bSuccess)
{
}

// TODO
void CConnection::OnDetached()
{
}

void CConnection::OnIncomingData()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    m_pIO->SetReadable();
    DoReceive();
    DoSend();
}

void CConnection::OnOutgoingReady()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    m_pIO->SetWritable();
    DoSend();
}

void CConnection::OnPeerClosed()
{
    TRACK_FUNCTION_LIFE_CYCLE;

    ErrorCode ec = EC_SUCCESS;
    if (m_pController && !m_pController->OnPeerClosed()) {
        ec = EC_PROTOCOL_ERROR;
    } else {
        CRequest* pRequest = reinterpret_cast<CRequest*>(m_WaitingRequests.First());
        if (pRequest) {
            if (!pRequest->OnPeerClosed()) {
                // Request is Not OK for this peer close.
                ec = EC_PROTOCOL_ERROR;
            }
            pRequest->OnTerminated(ec);
            m_WaitingRequests.PopFront();
        }
    }

    if (m_pIO->GetStatus() == CIOContext::IOS_UNREACHABLE) {
        ec = EC_CONNECT_FAILED;
    }
    m_Error = ec;

    if (IsIdle()) {
        m_RunContext.RemoveConnection(this);
        return;
    }

    OUTPUT_DEBUG_TRACE(
        "HTTP Connection is not idle: To Send: %d, To Receive: %d, Sending: %p\n",
        m_PendingRequests.Count(), m_WaitingRequests.Count(), m_pSendingRequest);
    if (ec != EC_CONNECT_FAILED) {
        m_Poller.PostAsynTask(ResendRequests, this);
    } else {
        Terminate(ec);
    }
}

bool CConnection::ActivateSecure()
{
    ASSERT(m_pIO);

    bool bRes = false;
    CSSLClient* pIO = CSSLClient::CreateInstance(m_pIO, false);
    if (pIO) {
        if (pIO->Open()) {
            bRes = true;
            m_pIO = pIO;
        } else {
            delete pIO;
        }
    }
    return bRes;
}

bool CConnection::TryPopRequest(CRequest* pReq)
{
    ASSERT(pReq);

    bool bRes = false;
    if (pReq != m_pSendingRequest) {
        CDuplexList::Iterator iter = m_PendingRequests.Begin();
        CDuplexList::Iterator iterEnd = m_PendingRequests.End();
        bool bFound = false;
        while (iter != iterEnd) {
            if (m_PendingRequests.DataAt(iter) == pReq) {
                bFound = true;
                break;
            }
            ++iter;
        }
        if (bFound) {
            m_PendingRequests.PopAt(iter);
            bRes = true;
        }
    }
    return bRes;
}

CRequest* CConnection::PrepareSendRequest()
{
    if (m_pSendingRequest) {
        return m_pSendingRequest;
    }

    CRequest* pLastRecvRequest =
        reinterpret_cast<CRequest*>(m_WaitingRequests.Last());
    if (pLastRecvRequest == NULL || pLastRecvRequest->IsPipeline()) {
        m_pSendingRequest = reinterpret_cast<CRequest*>(m_PendingRequests.First());
        if (m_pSendingRequest) {
            m_PendingRequests.PopFront();
        }
    }
    return m_pSendingRequest;
}

void CConnection::DoSend()
{
    while (m_pIO->IsWritable()) {
        size_t dataLength = m_pOutBuffer->GetDataLength();
        if (dataLength == 0) {
            ErrorCode error = EC_UNKNOWN;
            if (m_pController) {
                error = m_pController->GenerateData(
                    m_pOutBuffer->GetFreeBuffer(),
                    m_pOutBuffer->GetFreeBufferSize(),
                    &dataLength);
                if (error != EC_SUCCESS || error == EC_INPROGRESS) {
                    break;
                } else if (error == EC_INACTIVE) {
                    ASSERT(m_pIdleController == NULL);
                    m_pIdleController = m_pController;
                    m_pController = NULL;
                    error = EC_SUCCESS;
                }
            }
            if (dataLength == 0) {
                CRequest* pReq = PrepareSendRequest();
                if (pReq) {
                    if (pReq->Serialize(
                        m_pOutBuffer->GetFreeBuffer(),
                        m_pOutBuffer->GetFreeBufferSize(),
                        &dataLength)) {
                        // Serialize complete (filled to the buffer)
                        bool bRes = m_WaitingRequests.PushBack(pReq);
                        ASSERT(bRes);
                        m_pSendingRequest = NULL;
                    }
                }
            }
            if (dataLength == 0) {
                break;
            }
            m_pOutBuffer->SetPushInLength(dataLength);
        }
        if (!SendData()) {
            break;
        }
    }
}

bool CConnection::SendData()
{
    bool bRes = false;
    size_t writeBytes = m_pIO->Write(
        m_pOutBuffer->GetData(), m_pOutBuffer->GetDataLength());
    CIOContext::IOStatus writeStatus = m_pIO->GetStatus();
    switch (writeStatus) {
    case CIOContext::IOS_LOCAL_ERROR:
        HandleLocalError(ERROR_CODE);
        break;
    case CIOContext::IOS_CLOSED:
        // These errors should take by upper level.
        break;
    default:
        break;
    }
    if (writeBytes > 0) {
        m_pOutBuffer->SetPopOutLength(writeBytes, true);
        bRes = true;
    }
    return bRes;
}

void CConnection::DoReceive()
{
    while (m_pIO->IsReadable()) {
        size_t freeSize = m_pInBuffer->GetFreeBufferSize();
        if (freeSize == 0) {
            if (m_pInBuffer->GetTotalFreeSize() == 0) {
                // TODO: Handle the input buffer full.
                ASSERT(false);
                return;
            }
            m_pInBuffer->RelocationData();
            freeSize = m_pInBuffer->GetFreeBufferSize();
            ASSERT(freeSize > 0);
        }

        size_t dataLength =
            m_pIO->Read(m_pInBuffer->GetFreeBuffer(), freeSize);
        CIOContext::IOStatus readStatus = m_pIO->GetStatus();
        switch (readStatus) {
        case CIOContext::IOS_LOCAL_ERROR:
            HandleLocalError(ERROR_CODE);
            break;
        case CIOContext::IOS_CLOSED:
            // These errors should take by upper level.
            break;
        default:
            break;
        }

        if (dataLength > 0) {
            m_pInBuffer->SetPushInLength(dataLength);
            if (m_pInBuffer->GetDataLength() == 0) {
                break;
            }
            ReceiveData();
        }
    }
}

void CConnection::ReceiveData()
{
    size_t consumed = 0;
    uint8_t* pData = m_pInBuffer->GetData();
    size_t dataLen = m_pInBuffer->GetDataLength();
    ErrorCode error = EC_UNKNOWN;
    CRequest* pReq = reinterpret_cast<CRequest*>(m_WaitingRequests.First());

    do {
        size_t used = 0;

        // Step1: Handle the control data if has.
        if (m_pController) {
            error = m_pController->HandleData(pData, dataLen, &used);
            consumed += used;
            pData += used;
            dataLen -= used;
            if (error == EC_INPROGRESS) {
                continue;
            }
            if (error == EC_INACTIVE) {
                ASSERT(m_pIdleController == NULL);
                m_pIdleController = m_pController;
                m_pController = NULL;
                error = EC_SUCCESS;
            } else if (error != EC_SUCCESS) {
                break;
            }
        }

        // Step2: Handle the response data if has
        if (pReq) {
            error = pReq->OnResponse(pData, dataLen, &used);
            consumed += used;
            pData += used;
            dataLen -= used;
            if (error == EC_INPROGRESS) {
                continue;
            }
            m_WaitingRequests.PopFront();
            pReq->OnTerminated(error);
            pReq = reinterpret_cast<CRequest*>(m_WaitingRequests.First());
            if (error != EC_SUCCESS) {
                break;
            }
        } else {
            error = EC_UNKNOWN;
            break;
        }
    } while (consumed != 0 && dataLen != 0);

    if (error == EC_SUCCESS || error == EC_INPROGRESS) {
        m_pInBuffer->SetPopOutLength(consumed);
    } else {
        m_pInBuffer->Reset();
    }
}

bool CConnection::ResetIO()
{
    bool bSuccess = false;
    m_Poller.RemoveClient(this);
    m_pIO->Close();
    bSuccess = m_pIO->Open();
    if (bSuccess) {
        bSuccess = m_Poller.AddClient(this);
        if (!bSuccess) {
            m_pIO->Close();
        }
    }
    return bSuccess;
}

void CConnection::Terminate(ErrorCode ec)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CDuplexList::Iterator iter = m_WaitingRequests.Begin();
    CDuplexList::Iterator iterEnd = m_WaitingRequests.End();
    while (iter != iterEnd) {
        CRequest* pRequest =
            reinterpret_cast<CRequest*>(m_WaitingRequests.DataAt(iter));
        pRequest->OnTerminated(ec);
        ++iter;
    }
    iter = m_PendingRequests.Begin();
    iterEnd = m_PendingRequests.End();
    while (iter != iterEnd) {
        CRequest* pRequest =
            reinterpret_cast<CRequest*>(m_PendingRequests.DataAt(iter));
        pRequest->OnTerminated(ec);
        ++iter;
    }
    m_WaitingRequests.Reset();
    m_PendingRequests.Reset();
    m_RunContext.RemoveConnection(this);
}

void CConnection::HandleLocalError(int err /* = 0 */)
{
    ErrorCode ec = GetStandardErrorCode(err);
    ASSERT(ec != EC_SUCCESS);
    OUTPUT_WARNING_TRACE("HTTP connection local error: %s\n", GetErrorPhrase(ec));
    m_Error = ec;
    if (ec != EC_CONNECT_FAILED) {
        m_Poller.PostAsynTask(ResendRequests, this);
    } else {
        Terminate(ec);
    }
}

void CConnection::ResendRequests(void* pConn)
{
    TRACK_FUNCTION_LIFE_CYCLE;

    CConnection* pObject = reinterpret_cast<CConnection*>(pConn);
    ASSERT(pObject);

    if (pObject->m_pSendingRequest) {
        pObject->m_pSendingRequest->OnReset();
    }
    CDuplexList::Iterator iter = pObject->m_WaitingRequests.Begin();
    CDuplexList::Iterator iterEnd = pObject->m_WaitingRequests.End();
    while (iter != iterEnd) {
        CRequest* pRequest = reinterpret_cast<CRequest*>(
            pObject->m_WaitingRequests.DataAt(iter));
        pRequest->OnReset();
        if (!pObject->m_PendingRequests.PushBack(pRequest)) {
            pRequest->OnTerminated(pObject->m_Error);
        }
        ++iter;
    }
    pObject->m_WaitingRequests.Reset();
    CRequest* pSendingReq = pObject->m_pSendingRequest;
    if (pSendingReq) {
        pSendingReq->OnReset();
        if (!pObject->m_PendingRequests.PushBack(pSendingReq)) {
            pSendingReq->OnTerminated(pObject->m_Error);
        }
    }

    bool bToClosed = (pObject->m_PendingRequests.Count() == 0);
    if (!bToClosed) {
        ErrorCode error = EC_SUCCESS;
        if (pObject->ResetIO()) {
            if (pObject->m_pController) {
                pObject->m_pController->Reset();
            }
        } else {
            error = EC_IO_ERROR;
        }
        if (error != EC_SUCCESS) {
            CDuplexList::Iterator iterCur = pObject->m_PendingRequests.Begin();
            CDuplexList::Iterator iterLast = pObject->m_PendingRequests.End();
            while (iterCur != iterLast) {
                CRequest* pReq = reinterpret_cast<CRequest*>(
                    pObject->m_PendingRequests.DataAt(iterCur));
                pReq->OnTerminated(error);
                ++iterCur;
            }
            bToClosed = true;
        }
    }
    if (bToClosed) {
        pObject->m_RunContext.RemoveConnection(pObject);
    }
}

void CConnection::ReleaseBuffer(void* pBuf)
{
    if (pBuf) {
        free(pBuf);
    }
}
