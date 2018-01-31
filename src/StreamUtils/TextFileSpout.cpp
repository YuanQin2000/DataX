/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "TextFileSpout.h"
#include "Thread/ArrayDataFrames.h"
#include "Stream/MsgDefs.h"
#include <cstring>
#include <errno.h>
#include "Common/ErrorNo.h"
#include "Common/CharHelper.h"

using std::strerror;

CTextFileSpout::CTextFileSpout(
    const char* pDelimtor, tIOHandle fd /* = STDIN_FILENO */) :
    m_hIO(fd),
    m_Buffer(m_BufMem, sizeof(m_BufMem)),
    m_pDelimitor(pDelimtor),
    m_DelimitorLen(0),
    m_ScannedLen(0)
{
    ASSERT(fd != INVALID_IO_HANDLE);
    ASSERT(fd != STDOUT_FILENO);
    ASSERT(pDelimtor);

    m_DelimitorLen = strlen(pDelimtor);
    ASSERT(m_DelimitorLen > 0);
}

CTextFileSpout::~CTextFileSpout()
{
    close(m_hIO);
}

bool CTextFileSpout::Read(ArrayDataFrames** pOutData)
{
    ArrayDataFrames* pResult = NULL;
    ErrorCode errCode = EC_SUCCESS;
    bool bEof = false;
    size_t dataLen = m_Buffer.GetDataLength();

    while (dataLen < sizeof(m_BufMem)) {
        if (m_ScannedLen == dataLen) {
            // All bytes in buffer have been scanned, need more data to read.
            uint8_t* pBuf = m_Buffer.GetFreeBuffer();
            if (pBuf == NULL) {
                m_Buffer.RelocationData();
                pBuf = m_Buffer.GetFreeBuffer();
                ASSERT(pBuf);
            }
            size_t bufLen = m_Buffer.GetFreeBufferSize();
            int len = read(m_hIO, pBuf, bufLen);
            if (len == 0) {
                if (errno == 0) {
                    bEof = true;
                }
                break;
            } else if (len < 0) {
                errCode = GetStandardErrorCode(errno);
                bEof = (errCode == EC_IO_ERROR || errCode == EC_CONNECT_FAILED);
                OUTPUT_NOTICE_TRACE("read: %s\n", strerror(errno));
                break;
            }
            m_Buffer.SetPushInLength(len);
            dataLen += len;
        }

        char* pData = reinterpret_cast<char*>(m_Buffer.GetData());
        const char* pSectionEnd = NSCharHelper::FindSubStr(
            m_pDelimitor, m_DelimitorLen, pData + m_ScannedLen, pData + dataLen);
        if (pSectionEnd) {
            size_t sectionSize = pSectionEnd - pData + 1;
            pResult = ArrayDataFrames::CreateInstance(1);
            if (pResult) {
                DataFrame dataDesc(STREAM_USER_DATA);
                dataDesc.SetExtData(pData, sectionSize);
                pResult->SetFrame(0, &dataDesc, false);
                m_ScannedLen = 0;
                m_Buffer.SetPopOutLength(sectionSize);
            }
            break;
        }

        // Not found the delimitor
        m_ScannedLen = dataLen;
    }

    if (pResult == NULL && dataLen == sizeof(m_BufMem)) {
        // Buffer is full.
        OUTPUT_WARNING_TRACE("Buffer is full, force to process it.\n");
        pResult = ArrayDataFrames::CreateInstance(1);
        if (pResult) {
            DataFrame dataDesc(STREAM_USER_DATA);
            dataDesc.SetExtData(m_Buffer.GetData(), dataLen);
            pResult->SetFrame(0, &dataDesc, false);
        }
        m_ScannedLen = 0;
        m_Buffer.Reset();
    }
    *pOutData = pResult;
    return bEof;
}
