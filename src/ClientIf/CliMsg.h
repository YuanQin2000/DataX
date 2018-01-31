/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CLIENT_IF_MSG_H__
#define __CLIENT_IF_MSG_H__

#include <arpa/inet.h>
#include "Common/Typedefs.h"
#include "Common/Macros.h"
#include "Common/Vector.h"
#include "Tracker/Trace.h"

namespace NSCliMsg {

enum {
    MAX_PACKET_LENGTH = 4096,
    MAX_NAME_LENGTH = 255,
};

enum MsgIdentifier {
    MSG_IDENTIFIER_BEGIN = 0xC4,
    MSG_IDENTIFIER_REQ,
    MSG_IDENTIFIER_RESP,
    MSG_IDENTIFIER_QUERY,
    MSG_IDENTIFIER_INFO,
    MSG_IDENTIFIER_EVENT,
    MSG_IDENTIFIER_COUNT,
};

enum MsgRequestID {
    MSG_REQUEST_INVALID = -1,
    MSG_REQUEST_EXEC = 0,
    MSG_REQUEST_PUSH,
    MSG_REQUEST_POP,
    MSG_REQUEST_CLEAR,
    MSG_REQUEST_COUNT
};

enum MsgStatusCode {
    MSC_OK = 0,
    MSC_COMMAND_NOT_FOUND,
    MSC_PARAMETER_INVALID,
    MSC_SERVER_ERROR,
    MSC_COUNT
};

enum DataType {
    DT_CHAR_STRING = 0,
    DT_BYTE_DATA,
    DT_COUNT
};

enum BlockType {
    BT_COMMAND = 0,
    BT_VARIABLE,
    BT_ROOT_INFO,
    BT_COMMAND_INFO,
    BT_VARIABLE_INFO,
    BT_COUNT
};

inline bool IsValidIdentifier(uint8_t id)
{
    return id > MSG_IDENTIFIER_BEGIN && id < MSG_IDENTIFIER_COUNT;
}

inline bool IsValidRequestID(uint8_t reqID)
{
    return reqID >= 0 && reqID < MSG_REQUEST_COUNT;
}

inline bool IsValidStatusCode(uint8_t sc)
{
    return sc >= 0 && sc < MSC_COUNT;
}

inline bool IsValidDataType(uint8_t dt)
{
    return dt >= 0 && dt < DT_COUNT;
}

inline unsigned int Identifier2Index(uint8_t id)
{
    return static_cast<unsigned int>(id - MSG_IDENTIFIER_BEGIN - 1);
}

struct __ALIGN__(1) __PACKED__ Block {
    uint8_t Type;

    Block(uint8_t type) : Type(type) {}
};

struct __ALIGN__(1) __PACKED__ CommandDataBlock : public Block {
    uint8_t CmdID;

    CommandDataBlock(uint8_t id) : Block(BT_COMMAND), CmdID(id) {}
};

struct __ALIGN__(1) __PACKED__ VariableDataBlock : public Block {
    uint8_t DataType;
    uint16_t VarLength;
    uint8_t Data[0];

    VariableDataBlock(uint8_t dt, uint16_t len, const void* pData = NULL) :
        Block(BT_VARIABLE), DataType(dt), VarLength(htons(len))
    {
        ASSERT(IsValidDataType(dt));
        ASSERT(len > 0);
        if (pData != NULL) {
            memcpy(Data, pData, len);
        }
    }

    uint16_t GetVarLength() { return htons(VarLength); }
};

struct __ALIGN__(1) __PACKED__ RootInfoBlock : public Block {
    uint8_t SubCount;

    RootInfoBlock(uint8_t count) :
        Block(BT_ROOT_INFO), SubCount(count)
    {
        ASSERT(count > 0);
    }
};

struct __ALIGN__(1) __PACKED__ CommandInfoBlock : public Block {
    uint8_t CommandID;
    uint8_t SubCount;
    uint8_t NameLength;
    char    NameString[0];

    CommandInfoBlock(
        uint8_t cmdID,
        uint8_t count,
        uint8_t len,
        const char* pName = NULL) :
        Block(BT_COMMAND_INFO),
        CommandID(cmdID),
        SubCount(count),
        NameLength(len)
    {
        ASSERT(len > 0);
        if (pName != NULL) {
            memcpy(NameString, pName, NameLength);
        }
    }
};

struct __ALIGN__(1) __PACKED__ VariableInfoBlock : public Block {
    uint8_t SubCount;
    uint8_t BitSet;
    uint8_t NameLength;
    char    NameString[0];

    VariableInfoBlock(
        uint8_t count,
        bool bMustHave,
        bool bCharString,
        uint8_t len,
        const char* pName = NULL) :
        Block(BT_VARIABLE_INFO),
        SubCount(count),
        BitSet(0),
        NameLength(len)
    {
        ASSERT(len > 0);
        if (bMustHave) {
            SET_FLAG(BitSet, MANDATORY_FLAG);
        }
        if (bCharString) {
            SET_FLAG(BitSet, CHAR_STRING_FLAG);
        }
        if (pName != NULL) {
            memcpy(NameString, pName, NameLength);
        }
    }

    bool IsMandatory() const { return TEST_FLAG(BitSet, MANDATORY_FLAG); }
    bool IsCharString() const { return TEST_FLAG(BitSet, CHAR_STRING_FLAG); }

private:
    static const uint8_t MANDATORY_FLAG = 0x01;
    static const uint8_t CHAR_STRING_FLAG = 0x02;
};

struct __ALIGN__(1) __PACKED__ PayloadBase {
    uint16_t Length;

    PayloadBase(uint16_t len) :
        Length(htons(len)) { ASSERT(len >= sizeof(PayloadBase)); }

    uint16_t GetLength() { return ntohs(Length); }
    void SetLength(uint16_t len) { Length = htons(len); }
};

struct __ALIGN__(1) __PACKED__ ReqPayload : public PayloadBase {
    uint8_t RequestID;
    uint8_t Count;
    uint8_t Blocks[0];

    ReqPayload(uint16_t len, uint8_t reqID, uint8_t n) :
        PayloadBase(len + sizeof(ReqPayload)),
        RequestID(reqID),
        Count(n)
    {
        ASSERT(IsValidRequestID(reqID));   
    }
};

struct __ALIGN__(1) __PACKED__ RespPayload : public PayloadBase {
    uint8_t StatusCode;
    uint8_t BitSet;
    uint8_t Data[0];

    RespPayload(
        uint8_t status,
        uint16_t len = 0,
        uint8_t* pData = NULL,
        bool bLast = true) :
        PayloadBase(len + sizeof(RespPayload)),
        StatusCode(status),
        BitSet(0)
    {
        ASSERT(IsValidStatusCode(status));
        if (pData) {
            memcpy(Data, pData, len);
        }
        if (bLast) {
            SET_FLAG(BitSet, LAST_MESSAGE_FLAG);
        }
    }

    bool IsOK() const { return StatusCode == MSC_OK; }
    bool IsLast() const { return TEST_FLAG(BitSet, LAST_MESSAGE_FLAG); }

private:
    static const uint8_t LAST_MESSAGE_FLAG = 0x01;
};

struct __ALIGN__(1) __PACKED__ InfoPayload : public PayloadBase {
    uint8_t Count;
    uint8_t Blocks[0];

    InfoPayload(uint16_t len, uint8_t n) :
        PayloadBase(len + sizeof(InfoPayload)), Count(n) { ASSERT(n > 0); }
};

struct __ALIGN__(1) __PACKED__ Message {
    uint8_t Identifier;
    uint8_t BitSet;
    uint16_t SessionID;
    uint8_t Payload[0];

    Message(uint8_t id, uint16_t session, bool bHasPayload = true) :
        Identifier(id),
        BitSet(0),
        SessionID(htons(session))
    {
        ASSERT(IsValidIdentifier(id));
        if (bHasPayload) {
            SET_FLAG(BitSet, HAS_PAYLOAD_FLAG);
        }
    }

    void SetSession(uint16_t session) { SessionID = htons(session); }
    uint16_t GetSessionID() const { return ntohs(SessionID); }
    bool HasPayload() const { return TEST_FLAG(BitSet, HAS_PAYLOAD_FLAG); }
    size_t MessageLength() { return sizeof(Message) + PayloadLength(); }
    size_t PayloadLength()
    {
        return HasPayload() ?
               reinterpret_cast<PayloadBase*>(Payload)->GetLength() : 0;
    }

private:
    static const uint8_t HAS_PAYLOAD_FLAG = 0x01;
};


bool SetBlockIndex(ReqPayload* pRequest, CVector* pVector);
size_t GetBlockSize(Block* pBlock);

Message* DecodeMessage(
    uint8_t* pData, size_t len, size_t* pConsumedSize /* Output */);

const char* GetStatusPhrase(MsgStatusCode sc);
MsgRequestID GetRequestIDByName(const char* pStr);
const char* GetRequestNameByID(MsgRequestID reqID);
bool HasParam(MsgRequestID reqID);

void DumpBlock(Block* pBlock);
void DumpBlocks(CVector* pIndex);

}

#endif