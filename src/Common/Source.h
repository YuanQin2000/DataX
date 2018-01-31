/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __UTITLITY_PRODUCER_H__
#define __UTITLITY_PRODUCER_H__

#include <sys/epoll.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"
#include "IO/PollClient.h"

class CSink;
class CSource
{
public:
    /**
     * @brief Open the source for read
     * @param
     *      N/A
     * @return
     *      open result:
     *          true: Open successfully
     *          false: Open failed
     */
    virtual bool Open() = 0;

    /**
     * @brief Close the source
     * @param
     *      N/A
     * @return
     *      N/A
     */
    virtual void Close() = 0;

    /**
     * @brief Reset the source, then the Read will process from start position.
     * @param
     *      N/A
     * @return
     *      N/A
     */
    virtual void Reset() = 0;

    /**
     * @brief Read data from CSource to the buffer
     * @param
     *      to: Buffer
     *      size: Buffer Size
     * @return
     *      read status:
     *          < 0: Error.
     *          == 0: EOF.
     *          > 0: Actually read size.
     */
    virtual int Read(uint8_t* to, size_t size) = 0;

    /**
     * @brief Get Source Content Length
     * @param
     * @return
     *          < 0: Content Length is unknown (dynamic generated content)
     *          >= 0: Content Length
     */
    virtual int Length() const = 0;

    /**
     * @brief Return if the source content resource ready or not.
     *        if yes, then invoking "read" will not be blocked.
     */
    virtual bool IsReadable() const = 0;

    /**
     * @brief Return if reach the source content end.
     */
    virtual bool IsEOF() const = 0;

public:
    CSource();
    ~CSource();

    void OnConnected(CSink* pSink);
    void OnDisconnected(CSink* pSink);

protected:
    CSink* m_pSink;
};


class CMemorySource : public CSource
{
public:
    CMemorySource(uint8_t* data, size_t len);
    ~CMemorySource();

    bool Open()  { return true;          }
    void Close() { m_pCurrent = m_pData; }

    // From CSource
    int  Read(uint8_t* to, size_t size);
    int  Length() const { return m_Length; }
    bool IsReadable() const { return true; }
    bool IsEOF() const { return m_pCurrent == m_pData + m_Length; }

private:
    CMemorySource();

    uint8_t* m_pData;
    size_t   m_Length;
    uint8_t* m_pCurrent;
};

#endif
