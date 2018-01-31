/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HeaderParser.h"
#include <cctype>
#include "Common/ErrorNo.h"
#include "StatusLine.h"
#include "HeaderField.h"
#include "Memory/LazyBuffer.h"
#include "Common/CharHelper.h"
#include "Tracker/Trace.h"

using std::isspace;
using std::isalnum;

const char* CHeaderParser::s_pTokenDelimiters = "\"(),/:;<=>?@[\\]{}";

CHeaderParser::CHeaderParser(
    char* pData, size_t len, CLazyBuffer* pBuffer /* = NULL */) :
    m_pData(pData),
    m_DataLength(len),
    m_pBuffer(pBuffer),
    m_ConsumedSize(0)
{
    ASSERT(pData);
    ASSERT(len > 0);
}

CHeaderParser::~CHeaderParser()
{
}

void* CHeaderParser::BuildStartedLine(
    void* (*creator)(const char*, size_t, CLazyBuffer*))
{
    ASSERT(creator);

    const char* pCRLF =
        NSCharHelper::FindSubStr("\r\n", 2, m_pData, m_pData + m_DataLength);
    if (pCRLF == NULL) {
        // Need more data to parse.
        SET_ERROR_CODE(EC_INPROGRESS);
        return NULL;
    }

    size_t lineLen = pCRLF - m_pData;
    void* pLineObject = creator(m_pData, lineLen, m_pBuffer);
    m_ConsumedSize = lineLen + 2; // Add CRLF
    return pLineObject;
}

ErrorCode CHeaderParser::BuildHeaderField(CHeaderField* pHeaderField)
{
    ASSERT(pHeaderField);

    char* pCur = m_pData;
    char* pEnd = m_pData + m_DataLength;
    char* pCRLF = const_cast<char*>(
        NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd));
    ErrorCode resErr = EC_UNKNOWN;

    while (pCRLF) {
        char* pNewline = pCRLF + 2;
        if (pCRLF == pCur) {
            // Empty line with only CRLF, which means we reach the end of Header Field.
            m_ConsumedSize += 2;
            resErr = EC_SUCCESS;
            break;
        }

        /* 
        * field-name     = token
        * token          = 1*tchar
        * tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*" /
        *                  "+" / "-" / "." / "^" / "_" / "`" / "|" /
        *                  "~" / DIGIT / ALPHA
        *                  ; any VCHAR, except delimiters
        * 
        * field-value    = *( field-content / obs-fold )
        * field-content  = field-vchar [ 1*( SP / HTAB ) field-vchar ]
        * field-vchar    = VCHAR / obs-text
        * obs-fold       = CRLF 1*( SP / HTAB )
        */
        bool bSpaceBegin = false;
        while (pNewline < pEnd && (bSpaceBegin = (*pNewline == ' ' || *pNewline == '\t'))) {
            // Obsolete fold: Replace CRLF by SPs
            *pCRLF = ' ';
            *++pCRLF = ' ';
            pCRLF = const_cast<char*>(NSCharHelper::FindSubStr("\r\n", 2, pNewline, pEnd));
            if (!pCRLF) {
                // We need more data to identify if this obs-fold
                resErr = EC_INPROGRESS;
                break;
            }
            pNewline = pCRLF + 2;
        }
        if (pNewline == pEnd) {
            // We need more data to identify if this obs-fold
            resErr = EC_INPROGRESS;
            break;
        }

        m_ConsumedSize += pCRLF - pCur + 2;

        // A normal field value or reach the end of obs-fold.
        // Start to decode it.
        char* pName = NULL;
        char* pValue = NULL;
        resErr = DecodeHeaderField(pCur, pCRLF, &pName, &pValue);
        if (resErr != EC_SUCCESS) {
            OUTPUT_ERROR_TRACE(
                "DecodeHeaderField failed: %s\n", GetErrorPhrase(resErr));
            break;
        }

        if (pName && pValue) {
            pHeaderField->SetFieldValue(pName, pValue);
        } else {
            OUTPUT_WARNING_TRACE(
                "Header Field %s is NULL\n", pName ? "Name" : "Value");
            resErr = EC_PROTOCOL_MALFORMAT;
            break;
        }

        // Prepare the next
        pCur = pNewline;
        if (pCur < pEnd) {
            pCRLF = const_cast<char*>(NSCharHelper::FindSubStr("\r\n", 2, pCur, pEnd));
        }
    }

    return resErr;
}

// header-field   = field-name ":" OWS field-value OWS
ErrorCode CHeaderParser::DecodeHeaderField(
    char* pBegin, char* pEnd,
    char** pKey, char** pValue)
{
    char* pCur = pBegin;
    *pKey = *pValue = NULL;

    // Find Field Name.
    while (pCur < pEnd) {
        if (IsTokenChar(*pCur)) {
            ++pCur;
            continue;
        }
        if (*pCur != ':') {
            OUTPUT_WARNING_TRACE("Expect ':' while get %d\n", *pCur);
            return EC_PROTOCOL_MALFORMAT;
        }
        *pCur++ = '\0';
        break;
    }
    *pKey = pBegin;

    // Find Field Value
    while (pCur < pEnd) {
        // Skip the OWS before field value.
        if (!isspace(*pCur)) {  // TODO: isprint?
            break;
        }
        ++pCur;
    }
    if (pCur == pEnd) { // Empty value.
        return EC_SUCCESS;
    }

    // Found the start of value.
    *pValue = pCur;
    ++pCur;
    *pEnd = '\0';
    while (pEnd > pCur) {
        // Skip the OWS after field value.
        if (!isspace(*pEnd)) {  // TODO: isprint?
            break;
        }
        *pEnd-- = '\0';
    }
    return EC_SUCCESS;
}

bool CHeaderParser::IsTokenChar(char c)
{
    return isalnum(c) ||
           c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
           c == '\'' || c =='*' || c == '+' || c == '-' || c == '.' ||
           c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
}
