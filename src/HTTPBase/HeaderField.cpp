/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "HeaderField.h"
#include <cstdio>
#include <new>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include "FieldValue.h"
#include "Common/StringPicker.h"
#include "Tracker/Trace.h"

using std::malloc;
using std::free;
using std::memset;
using std::memcpy;


///////////////////////////////////////////////////////////////////////////////
//
// CHeaderField Implemenation
//
///////////////////////////////////////////////////////////////////////////////
int CHeaderField::GlobalConfig::GetFieldID(const char* pName) const
{
    ASSERT(pName);

    tStringIDMap::const_iterator iter = pNamesMap->find(pName);
    if (iter != pNamesMap->end()) {
        return iter->second;
    }
    return INVALID_FIELD_ID;
}

CHeaderField::FieldInitedValue::FieldInitedValue(
    CMemory& mem, size_t count /* = 0 */) : ItemCount(count)
{
    for (size_t i = 0; i < count; ++i) {
        new (&Values[i].ValueObjects) CForwardList(&mem);
    }
}

void CHeaderField::FieldInitedValue::Append(size_t index, IFieldValue* pValue)
{
    ASSERT(ItemCount > 0);

    int insertPos = -1;
    for (size_t i = 0; i < ItemCount; ++i) {
        if (Values[i].ValueObjects.Count() == 0 || Values[i].Index == index) {
            insertPos = static_cast<int>(i);
            break;
        }
    }
    ASSERT(insertPos >= 0);
    Values[insertPos].Index = index;
    Values[insertPos].ValueObjects.PushBack(pValue);
}

void CHeaderField::FieldInitedValue::Clear(size_t index)
{
    for (size_t i = 0; i < ItemCount; ++i) {
        if (Values[i].Index == index) {
            Values[i].ValueObjects.Reset();
            return;
        }
    }
}

CForwardList* CHeaderField::FieldInitedValue::GetValue(size_t index)
{
    for (size_t i = 0; i < ItemCount; ++i) {
        if (Values[i].Index == index) {
            return &Values[i].ValueObjects;
        }
    }
    return NULL;
}


CHeaderField::CHeaderField(CLazyBuffer& buffer, const GlobalConfig* pCfg) :
    m_pConfig(pCfg),
    m_pValues(NULL),
    m_Extensions(),
    m_Buffer(buffer)
{
    ASSERT(pCfg);
    ASSERT(pCfg->ItemsCount > 0);
    ASSERT(pCfg->pItems);
    ASSERT(pCfg->pNamesMap);
}

CHeaderField::~CHeaderField()
{
    if (m_pValues) {
        free(m_pValues);
        m_pValues = NULL;
    }
}

CHeaderField* CHeaderField::CreateInstance(
    const GlobalConfig* pCfg,
    CLazyBuffer& buffer,
    FieldInitedValue* pInitValues /* = NULL */)
{
    CHeaderField* pInstance = new CHeaderField(buffer, pCfg);
    if (pInstance == NULL) {
        return NULL;
    }

    size_t size = pCfg->ItemsCount * sizeof(CForwardList*);
    pInstance->m_pValues = reinterpret_cast<CForwardList**>(malloc(size));
    if (pInstance->m_pValues) {
        memset(pInstance->m_pValues, 0, size);
        if (pInitValues) {
            for (size_t i = 0; i < pInitValues->ItemCount; ++i) {
                if (pInitValues->Values[i].ValueObjects.Count() == 0) {
                    continue;
                }
                pInstance->SetFieldValueByID(
                    pInitValues->Values[i].Index,
                    &pInitValues->Values[i].ValueObjects);
            }
        }
    } else {
        delete pInstance;
        pInstance = NULL;
    }
    return pInstance;
}

CForwardList* CHeaderField::GetFieldValueByName(const char* pName)
{
    ASSERT(pName);

    int fieldID = m_pConfig->GetFieldID(pName);
    if (fieldID >= 0) {
        return m_pValues[fieldID];
    }
    return NULL;
}

size_t CHeaderField::Count() const
{
    size_t count = 0;
    for (size_t i = 0; i < m_pConfig->ItemsCount; ++i) {
         if (m_pValues[i] != NULL) {
             ++count;
         }
    }
    return count;
}

bool CHeaderField::SetExtFieldValue(const char* pName, const char* pValue)
{
    ASSERT(pName);
    ASSERT(pValue);

    if (!m_pConfig->bSupportExtension) {
        return false;
    }
    int fieldID = m_pConfig->GetFieldID(pName);
    if (fieldID >= 0) {
        return false;
    }
    return m_Extensions.SetValue(pName, pValue);
}

bool CHeaderField::SetFieldValue(const char* pName, const char* pValue)
{
    ASSERT(pName);
    ASSERT(pValue);

    int fieldID = m_pConfig->GetFieldID(pName);
    if (fieldID < 0) {
        bool bRes = false;
        if (m_pConfig->bSupportExtension) {
            bRes = m_Extensions.SetValue(pName, pValue);
        }
        return bRes;
    }

    if (!m_pConfig->pItems[fieldID].SupportMultiItems() && m_pValues[fieldID]) {
        OUTPUT_WARNING_TRACE("Set item is existed: %s\n", pName);
        return false;
    }

    tValueCreator creatorFunc = m_pConfig->pItems[fieldID].CreatorFunction;
    if (creatorFunc == NULL) {
        OUTPUT_WARNING_TRACE("No create function for header field: %s\n", pValue);
        return false;
    }

    CForwardList* pValueObjects = CreateFieldValueList(
        pValue,
        m_pConfig->pItems[fieldID].DelimitedChar,
        creatorFunc,
        m_Buffer);
    if (pValueObjects == NULL) {
        return false;
    }
    if (m_pValues[fieldID]) {
        m_pValues[fieldID]->PushBack(*pValueObjects);
    } else {
        m_pValues[fieldID] = pValueObjects;
    }
    return true;
}

bool CHeaderField::SetFieldValue(int fieldID, const char* pValue)
{
    ASSERT(fieldID >= 0);
    ASSERT(pValue);

    if (!m_pConfig->pItems[fieldID].SupportMultiItems() && m_pValues[fieldID]) {
        OUTPUT_WARNING_TRACE("Set item is existed: %d\n", fieldID);
        return false;
    }
    tValueCreator creatorFunc = m_pConfig->pItems[fieldID].CreatorFunction;
    if (creatorFunc == NULL) {
        OUTPUT_WARNING_TRACE("No create function for header field: %s\n", pValue);
        return false;
    }

    CForwardList* pValueObjects = CreateFieldValueList(
        pValue,
        m_pConfig->pItems[fieldID].DelimitedChar,
        creatorFunc,
        m_Buffer);
    if (pValueObjects == NULL) {
        return false;
    }
    if (m_pValues[fieldID]) {
        m_pValues[fieldID]->PushBack(*pValueObjects);
    } else {
        m_pValues[fieldID] = pValueObjects;
    }
    return true;
}

bool CHeaderField::SetFieldValueByID(int fieldID, CForwardList* pValues)
{
    ASSERT(pValues);
    ASSERT(fieldID >= 0);
    ASSERT(static_cast<unsigned int>(fieldID) < m_pConfig->ItemsCount);

    if (m_pValues[fieldID]) {
        if (!m_pConfig->pItems[fieldID].SupportMultiItems()) {
            OUTPUT_WARNING_TRACE("Set item is existed: %d\n", fieldID);
            return false;
        }
        m_pValues[fieldID]->PushBack(*pValues);
    } else {
        m_pValues[fieldID] = pValues;
    }
    return true;
}

bool CHeaderField::AppendFieldValue(int fieldID, IFieldValue* pValue)
{
    ASSERT(pValue);
    ASSERT(fieldID >= 0);
    ASSERT(static_cast<unsigned int>(fieldID) < m_pConfig->ItemsCount);

    CForwardList* pList = m_pValues[fieldID];
    if (pList == NULL) {
        void* pMem = m_Buffer.Malloc(sizeof(CForwardList));
        if (pMem) {
            pList = new (pMem) CForwardList(&m_Buffer);
        }
    }
    if (pList) {
        return pList->PushBack(pValue);
    }
    return false;
}

CHeaderField::tSerializeAnchor CHeaderField::Serialize(
    char* pBuffer,
    size_t len,
    const char* pKeyValueDelimiter,
    const char* pItemDelimiter,
    tSerializeAnchor anchor,
    size_t* pOutLen)
{
    ASSERT(pBuffer);
    ASSERT(len > 0);
    ASSERT(pKeyValueDelimiter);
    ASSERT(*pKeyValueDelimiter != '\0');
    ASSERT(pItemDelimiter);
    ASSERT(*pItemDelimiter != '\0');

    size_t copied = 0;
    size_t availCopied = 0;
    bool bFinished = true;
    char* pCur = pBuffer;
    const char* pEnd = pBuffer + len;

    *pOutLen = 0;
    size_t curIndex;
    for (curIndex = anchor; curIndex < m_pConfig->ItemsCount; ++curIndex) {
        CForwardList* pValues = m_pValues[curIndex];
        if (pValues == NULL) {
            continue;
        }
        ASSERT(pValues->Count() > 0);

        CForwardList::Iterator iter = pValues->Begin();
        CForwardList::Iterator iterEnd = pValues->End();
        do {
            // Copy the Name string.
            const ItemConfig* pItemConfig = &m_pConfig->pItems[curIndex];
            bFinished = NSCharHelper::CopyNChars(
                pCur, pEnd - pCur, pItemConfig->pItemName, &copied);
            pCur += copied;
            if (!bFinished || pCur == pEnd) {
                return curIndex;
            }
            availCopied += copied;

            // Copy Separator String
            bFinished = NSCharHelper::CopyNChars(
                pCur, pEnd - pCur, pKeyValueDelimiter, &copied);
            pCur += copied;
            if (!bFinished || pCur == pEnd) {
                return curIndex;
            }
            availCopied += copied;

            while (true) {
                IFieldValue* pFV = reinterpret_cast<IFieldValue*>(pValues->DataAt(iter));
                bFinished = pFV->Print(pCur, pEnd - pCur, &copied);
                pCur += copied;
                availCopied += copied;
                if (!bFinished || pCur == pEnd) {
                    return curIndex;
                }
                ++iter;
                if (iter == iterEnd || pItemConfig->DelimitedChar == '\0') {
                    // New items.
                    // Or multiple items, use the delimitor string instead.
                    break;
                }

                // Copy the delimitor character
                *pCur++ = pItemConfig->DelimitedChar;
                if (pCur == pEnd) {
                    return curIndex;
                }
                ++availCopied;
            }

            // Append the delimitor string to the key-value string
            bFinished = NSCharHelper::CopyNChars(
                    pCur, pEnd - pCur, pItemDelimiter, &copied);
            pCur += copied;
            availCopied += copied;
            if (!bFinished || pCur == pEnd) {
                return curIndex;
            }
        } while (iter != iterEnd);
        *pOutLen = availCopied;
    }
    bFinished = m_Extensions.Print(
        pCur, pEnd - pCur, pKeyValueDelimiter, pItemDelimiter, &copied);
    if (bFinished) {
        ++curIndex;
        *pOutLen += copied;
    }
    return curIndex;
}

CForwardList* CHeaderField::CreateFieldValueList(
    const char* pString,
    char delimitor,
    tValueCreator creator,
    CLazyBuffer& buffer)
{
    ASSERT(pString);
    ASSERT(creator);

    void* pMem = buffer.Malloc(sizeof(CForwardList));
    if (pMem == NULL) {
        OUTPUT_WARNING_TRACE("Create List Head failed.\n");
        return NULL;
    }
    CForwardList* pList = new (pMem) CForwardList(&buffer);
    CStringPicker picker(delimitor, pString);
    const char* pSubString = NULL;
    size_t subStringLen = 0;
    bool bHasNext = picker.GetSubString(&pSubString, &subStringLen);
    while (pSubString) {
        IFieldValue* pInstance = creator(pSubString, subStringLen, buffer);
        if (pInstance) {
            pList->PushBack(pInstance);
        } else {
            OUTPUT_WARNING_TRACE(
                "Create Field Vaule failed on: %s (len: %d)\n", pSubString, subStringLen);
            break;
        }
        if (bHasNext) {
            bHasNext = picker.GetSubString(&pSubString, &subStringLen);
        } else {
            pSubString = NULL;
            subStringLen = 0;
        }
    }
    return pList->Count() > 0 ? pList : NULL;
}

void CHeaderField::Dump()
{
    char buffer[2048];
    size_t printLen = 0;
    tSerializeAnchor anchor = AnchorBegin();

    OUTPUT_RAW_TRACE("===============================================\n");
    while (!IsEnd(anchor)) {
        anchor = Serialize(
            buffer,
            sizeof(buffer) - 1,
            ": ", "\n",
            anchor,
            &printLen);
        buffer[printLen] = '\0';
        OUTPUT_RAW_TRACE("%s\n", buffer);
    }
    OUTPUT_RAW_TRACE("===============================================\n");
}
