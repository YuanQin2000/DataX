/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATA_COM_REQURST_H__
#define __DATA_COM_REQURST_H__

#include "Common/Typedefs.h"
#include "Common/ErrorNo.h"
#include "Common/Macros.h"

class CConfigure;
class CIOContext;
struct sockaddr;

class CRequest
{
public:
    virtual ~CRequest() {}

    virtual bool Serialize(uint8_t* pBuf, size_t bufLen, size_t* pOutLen) = 0;
    virtual ErrorCode OnResponse(uint8_t* pData, size_t dataLen, size_t* pConsumedLen) = 0;
    virtual ErrorCode OnPeerClosed() = 0;
    virtual void OnTerminated(ErrorCode err) = 0;
    virtual void OnReset() = 0;

    virtual CIOContext* CreateIOContext(const sockaddr* pTarget) = 0;
    virtual CConfigure& GetConfigure() = 0;

    bool HasResponse() const { return TEST_FLAG(m_Flags, HAS_RESPONSE_FLAG); }
    bool IsPipeline() const { return TEST_FLAG(m_Flags, SUPPORT_PIPE_LINE_FLAG); }

protected:
    CRequest(bool bHasResp, bool bPipeline) : m_Flags(0)
    {
        if (bHasResp) {
            SET_FLAG(m_Flags, HAS_RESPONSE_FLAG);
        }
        if (bPipeline) {
            SET_FLAG(m_Flags, SUPPORT_PIPE_LINE_FLAG);
        }
    }

private:
    uint8_t m_Flags;

    static const uint8_t HAS_RESPONSE_FLAG = 0x01;
    static const uint8_t SUPPORT_PIPE_LINE_FLAG = 0x02;

    DISALLOW_COPY_CONSTRUCTOR(CRequest);
    DISALLOW_ASSIGN_OPERATOR(CRequest);
    DISALLOW_DEFAULT_CONSTRUCTOR(CRequest);
};

#endif