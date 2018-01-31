/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __STREAM_RUNNER_H__
#define __STREAM_RUNNER_H__

#include "Common/Typedefs.h"
#include "Common/DiGraph.h"
#include "BoltChain.h"


struct ArrayDataFrames;

/**
 * @brief CStreamRunner
 * @warning Not thread safe
 */
class CStreamRunner : public CDGNode
{
public:
    virtual ~CStreamRunner();

    virtual bool Start() = 0;
    virtual bool Stop() = 0;

    /**
     * @brief Forward the message to the downstream node.
     * @param pMsg the forwarded message.
     * @return The count of message (downstream) forwarded.
     * @warning The @pMsg will be transferred the ownship to the downstream
     */
    size_t ForwardMessage(ArrayDataFrames* pMsg);

    /**
     * @warning The ownership of @pInData will be transferred to process
     *          as the bolt chain does
     */
    ArrayDataFrames* Process(ArrayDataFrames* pInData)
    {
        if (m_pBoltChain) {
            return m_pBoltChain->Process(pInData);
        }
        return pInData;
    }

    const char* GetName() const { return m_Name; }

protected:
    CStreamRunner(const char* pName, CBoltChain* pChain);

private:
    CBoltChain* m_pBoltChain;   // Owned
    char m_Name[32];

    DISALLOW_DEFAULT_CONSTRUCTOR(CStreamRunner);
    DISALLOW_COPY_CONSTRUCTOR(CStreamRunner);
    DISALLOW_ASSIGN_OPERATOR(CStreamRunner);
};

#endif