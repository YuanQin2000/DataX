/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "RemoteBlot.h"
#include "DataCom/Connection.h"
#include "Common/ByteData.h"
#include "Thread/ArrayDataFrames.h"

ArrayDataFrames* CRemoteBolt::Process(ArrayDataFrames* pInData)
{
    CByteData byteData(pInData, 0, ArrayDataFrames::DeleteInstance);
    m_Connection.PushRequest(&byteData);
    return NULL;
}
