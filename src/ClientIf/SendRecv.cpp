/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "SendRecv.h"
#include <cstdlib>
#include "IO/IOContext.h"
#include "Tracker/Trace.h"

using std::malloc;

namespace NSSendRecv
{

bool Send(NSCliMsg::Message* pMsg, CIOContext& io)
{
    size_t msgLen = pMsg->MessageLength();
    size_t writeLen = io.Write(pMsg, msgLen);
    return msgLen == writeLen;
}

// TODO: Timeout
NSCliMsg::Message* Recv(uint16_t session, CIOContext& io, int timeout /* = -1 */)
{
    uint8_t buffer[sizeof(NSCliMsg::Message) + sizeof(NSCliMsg::PayloadBase)];
    size_t hdrLen = io.Read(buffer, sizeof(NSCliMsg::Message));
    if (hdrLen != sizeof(NSCliMsg::Message)) {
        OUTPUT_ERROR_TRACE(
            "Read message header failed: expected %d received %d\n",
            sizeof(NSCliMsg::Message), hdrLen);
        return NULL;
    }

    NSCliMsg::Message* pMsg = NULL;
    if (!reinterpret_cast<NSCliMsg::Message*>(buffer)->HasPayload()) {
        pMsg = reinterpret_cast<NSCliMsg::Message*>(malloc(sizeof(NSCliMsg::Message)));
        if (pMsg) {
            memcpy(pMsg, buffer, sizeof(NSCliMsg::Message));
        }
        return pMsg;
    }

    // Read the length first.
    size_t plLen = io.Read(buffer + sizeof(NSCliMsg::Message),
                           sizeof(NSCliMsg::PayloadBase));
    if (plLen != sizeof(NSCliMsg::PayloadBase)) {
        OUTPUT_ERROR_TRACE(
            "Read message length failed: expected %d received %d\n",
            sizeof(NSCliMsg::PayloadBase), plLen);
        return NULL;
    }

    size_t msgLen = reinterpret_cast<NSCliMsg::Message*>(buffer)->MessageLength();
    uint8_t* pMsgBuf = reinterpret_cast<uint8_t*>(malloc(msgLen));
    if (pMsgBuf == NULL) {
        OUTPUT_ERROR_TRACE("Allocate memory failed: %d\n", msgLen);
        return NULL;
    }

    memcpy(pMsgBuf, buffer, sizeof(buffer));
    size_t expectedLen = msgLen - sizeof(buffer);
    size_t payloadLen = io.Read(pMsgBuf + sizeof(buffer), expectedLen);
    if (expectedLen != payloadLen) {
        OUTPUT_ERROR_TRACE("Read message body failed: expected %d received %d\n",
                            expectedLen, payloadLen);
        free(pMsgBuf);
        return NULL;
    }

    pMsg = reinterpret_cast<NSCliMsg::Message*>(pMsgBuf);
    if (pMsg->GetSessionID() != session) {
        OUTPUT_ERROR_TRACE(
            "Discard Message since session is not right: expected %u received %u",
            session, pMsg->GetSessionID());
        free(pMsgBuf);
        return NULL;
    }
    return pMsg;
}


// TODO: Support timeout.
NSCliMsg::Message* Recv(
    uint16_t session,
    CIOContext& io,
    uint8_t* pBuf,
    size_t len,
    int timeout /* = -1, milliseconds */)
{
    if (len < sizeof(NSCliMsg::Message)) {
        OUTPUT_ERROR_TRACE("Buffer is too small (%d) to fill the message header\n", len);
        return NULL;
    }
    size_t hdrLen = io.Read(pBuf, sizeof(NSCliMsg::Message));
    if (hdrLen != sizeof(NSCliMsg::Message)) {
        OUTPUT_ERROR_TRACE("Read message body failed: expected %d received %d\n",
                           sizeof(NSCliMsg::Message), hdrLen);
        return NULL;
    }

    NSCliMsg::Message* pMsg = reinterpret_cast<NSCliMsg::Message*>(pBuf);
    if (!pMsg->HasPayload()) {
        return pMsg;
    }

    // Read the length first.
    size_t plLen = io.Read(
        pBuf + sizeof(NSCliMsg::Message), sizeof(NSCliMsg::PayloadBase));
    if (plLen != sizeof(NSCliMsg::PayloadBase)) {
        OUTPUT_ERROR_TRACE(
            "Read message length failed: expected %d received %d\n",
            sizeof(NSCliMsg::PayloadBase), plLen);
        return NULL;
    }

    size_t msgLen = pMsg->MessageLength();
    if (msgLen > len) {
        OUTPUT_ERROR_TRACE("Buffer is too small (%d) to fill whole message.\n", len);
        return NULL;
    }
    if (msgLen > sizeof(NSCliMsg::Message)) {
        size_t expectedLen = msgLen -
                             sizeof(NSCliMsg::Message) -
                             sizeof(NSCliMsg::PayloadBase);
        size_t payloadLen = io.Read(
            pBuf + sizeof(NSCliMsg::Message) + sizeof(NSCliMsg::PayloadBase), expectedLen);
        if (expectedLen != payloadLen) {
            OUTPUT_ERROR_TRACE("Read message body failed: expected %d received %d\n",
                               expectedLen, payloadLen);
            return NULL;
        }
    }
    if (pMsg->GetSessionID() != session) {
        OUTPUT_ERROR_TRACE(
            "Discard Message since session is not right: expected %u received %u",
            session, pMsg->GetSessionID());
        return NULL;
    }
    return pMsg;
}

NSCliMsg::Message* SendAndRecv(
    NSCliMsg::Message* pMsg,
    CIOContext& io,
    uint8_t* pBuf /* = NULL */,
    size_t len /* = 0 */,
    int timeout /* = -1, milliseconds */)
{
    if (!Send(pMsg, io)) {
        OUTPUT_ERROR_TRACE("Send message failed.\n");
        return NULL;
    }

    if (pBuf == NULL && len == 0) {
        return Recv(pMsg->GetSessionID(), io, timeout);
    }
    if (pBuf != NULL && len > 0) {
        return Recv(pMsg->GetSessionID(), io, pBuf, len, timeout);
    }
    ASSERT(false);
    return NULL;
}

};