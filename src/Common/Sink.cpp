/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Sink.h"
#include "Source.h"
#include "Tracker/Trace.h"

CSink::CSink() : m_pSource(NULL)
{
}

CSink::~CSink()
{
}

void CSink::ConnectSource(CSource* pSource)
{
    ASSERT(pSource);
    ASSERT(!m_pSource);

    m_pSource = pSource;
    pSource->OnConnected(this);
}

void CSink::DisconnectSource()
{
    ASSERT(m_pSource);

    m_pSource->OnDisconnected(this);
    m_pSource = NULL;
}
