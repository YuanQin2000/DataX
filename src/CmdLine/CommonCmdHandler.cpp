/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CommonCmdHandler.h"
#include "Common/Macros.h"
#include "CommandLine.h"
#include "LineArguments.h"
#include "Common/StringIndex.h"
#include "Tracker/Trace.h"

const char* CCommonCmdHandler::s_CommandNames[CID_COUNT] = {
    "QUIT"
};

const CCommonCmdHandler::tCmdHandler
CCommonCmdHandler::s_CommandHandlers[CID_COUNT] = {
    HandleQuitCommand,
};

const CStringIndex* CCommonCmdHandler::s_NameIndex = CreateCommandNameIndex();

CCommonCmdHandler::CCommonCmdHandler() : CCmdHandler()
{
}

CCommonCmdHandler::~CCommonCmdHandler()
{
}

bool CCommonCmdHandler::HandleCommand(CLineArguments* pArgs)
{
    CLineArguments::ArgumentData* pCmdData = pArgs->GetArgumentAt(0);
    bool bConsumed = false;
    if (pCmdData->Type == CLineArguments::AT_STRING) {
        tCmdHandler handler = GetBuiltinCmdHandler(pCmdData->pString);
        if (handler) {
            bConsumed = true;
            handler(this, pArgs);
        }
    }
    return bConsumed;
}

CCommonCmdHandler::tCmdHandler CCommonCmdHandler::GetBuiltinCmdHandler(const char* pName)
{
    tCmdHandler handler = NULL;
    if (s_NameIndex) {
        int index = s_NameIndex->GetIndexByString(pName);
        if (index >= 0) {
            handler = s_CommandHandlers[index];
        }
    }
    return handler;
}

void CCommonCmdHandler::HandleQuitCommand(CCommonCmdHandler* pThis, CLineArguments* pArgs)
{
    ASSERT(pThis);
    ASSERT(pArgs);

    if (pArgs->ArgumentsCount() > 1) {
        pThis->m_bQuit = false;
        pThis->m_Error = CEC_PARAM_INVALID;
    } else {
        pThis->m_bQuit = true;
        pThis->m_Error = CEC_OK;
    }
}

CStringIndex* CCommonCmdHandler::CreateCommandNameIndex()
{
    CStringIndex* pInstance = CStringIndex::CreateInstance(
        COUNT_OF_ARRAY(s_CommandNames), s_CommandNames, false);
    if (pInstance == NULL) {
        OUTPUT_ERROR_TRACE("Create Command Name Index failed\n");
    }
    return pInstance;
}
