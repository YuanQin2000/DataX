/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __DATA_COM_CONTROLLER_H__
#define __DATA_COM_CONTROLLER_H__

#include "Common/ErrorNo.h"
#include "Tracker/Trace.h"

class CRequest;
class CConnection;
class CController
{
public:
    virtual ~CController() {}

    /**
     * @brief Handle received data.
     * @param pData. The received data.
     * @param dataLen Data length.
     * @param pConsumedLen output, Consumed length in this call.
     * @return error code. 
     */
    virtual ErrorCode HandleData(uint8_t* pData, size_t dataLen, size_t* pConsumedLen) = 0;

    /**
     * @brief Generate control data.
     * @param pBuf The buffer for store the control data.
     * @param bufLen The buffer length
     * @param pOutLen The generated data length.
     * @return Return status code, @see ErrorCode definition.
     *          EC_INPROGRESS: no error, in progress.
     *          EC_SUCCESS: no error, complete.
     *          Error code.
     */
    virtual ErrorCode GenerateData(uint8_t* pBuf, size_t bufLen, size_t* pOutLen) = 0;

    virtual bool OnPeerClosed() = 0;
    virtual void Reset() = 0;

    void SetConnection(CConnection* pConn)
    {
        ASSERT(pConn);
        ASSERT(m_pConnect == NULL);
        m_pConnect = pConn;
    }

protected:
    CConnection* m_pConnect;    // Not owned
};

#endif