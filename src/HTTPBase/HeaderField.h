/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTP_BASE_HEADER_FIELD_H__
#define __HTTP_BASE_HEADER_FIELD_H__

#include <map>
#include <cstring>
#include "Common/Macros.h"
#include "Memory/LazyBuffer.h"
#include "Common/ForwardList.h"
#include "Common/StringMap.h"
#include "Tracker/Trace.h"

using std::strchr;
using std::map;
using std::pair;
using std::memcmp;

class IFieldValue;

///////////////////////////////////////////////////////////////////////////////
//
// CHeaderField Definition
//
///////////////////////////////////////////////////////////////////////////////

typedef map<const char*, int, tStringCompareFunc> tStringIDMap;
typedef IFieldValue* (*tValueCreator)(
        const char* pString, size_t len, CLazyBuffer& buffer);


class CHeaderField
{
public:
    struct ItemConfig {
        const char* pItemName;
        uint8_t BitSet;
        char DelimitedChar;     // If mulitple values enable
        tValueCreator CreatorFunction;

        static const uint8_t MULTI_VALUES_FLAG = 0x01;
        static const uint8_t MULTI_ITEMS_FLAG = 0x02;

        bool SupportMultiValues() const { return TEST_FLAG(BitSet, MULTI_VALUES_FLAG); }
        bool SupportMultiItems() const { return TEST_FLAG(BitSet, MULTI_ITEMS_FLAG); }
    };

    struct GlobalConfig {
        bool bSupportExtension;
        size_t ItemsCount;
        const ItemConfig* pItems;
        const tStringIDMap* pNamesMap;

        GlobalConfig(
            bool bSupExt,
            size_t count,
            const ItemConfig* pConf,
            const tStringIDMap* pMap) :
            bSupportExtension(bSupExt),
            ItemsCount(count),
            pItems(pConf),
            pNamesMap(pMap) {}

        int GetFieldID(const char* pName) const;
    };

    struct IndexValueHead {
        size_t Index;
        CForwardList ValueObjects;

        IndexValueHead() : Index(0), ValueObjects() {}
    };

    struct FieldInitedValue {
        size_t ItemCount;
        IndexValueHead Values[0];

        FieldInitedValue(CMemory& mem, size_t count = 0);
        void Append(size_t index, IFieldValue* pValue);
        void Clear(size_t index);
        CForwardList* GetValue(size_t index);
    };

public:
    ~CHeaderField();

    CForwardList* GetFieldValueByName(const char* pName);
    CForwardList* GetFieldValueByID(int fieldID)
    {
        ASSERT(fieldID > 0);
        ASSERT(static_cast<unsigned int>(fieldID) < m_pConfig->ItemsCount);
        return m_pValues[fieldID];
    }

    size_t Count() const;
    const CStringMap& GetExtensions() const { return m_Extensions; }
    CLazyBuffer& GetBuffer() { return m_Buffer; }

    bool SetFieldValue(const char* pName, const char* pValue);
    bool SetFieldValue(int fieldID, const char* pValue);
    bool SetExtFieldValue(const char* pName, const char* pValue);
    bool SetFieldValue(int fieldID, CForwardList* pValues)
    {
        return SetFieldValueByID(fieldID, pValues);
    }

    static CHeaderField* CreateInstance(
        const GlobalConfig* pCfg,
        CLazyBuffer& buffer,
        FieldInitedValue* pInitValues = NULL);

public:
    typedef size_t tSerializeAnchor;
    tSerializeAnchor Serialize(
        char* pBuffer,
        size_t bufLen,
        const char* pKeyValueDelimiter,
        const char* pItemDelimiter,
        tSerializeAnchor anchor,
        size_t* pOutLen);
    tSerializeAnchor AnchorBegin() const { return 0; }
    bool IsEnd(tSerializeAnchor anchor) const { return anchor > m_pConfig->ItemsCount; }

    void Dump();

private:
    CHeaderField(CLazyBuffer& buffer, const GlobalConfig* pCfg);

    bool SetFieldValueByID(int fieldID, CForwardList* pValues);
    bool AppendFieldValue(int fieldID, IFieldValue* pValue);

    static CForwardList* CreateFieldValueList(
        const char* pString,
        char delimitor,
        tValueCreator creator,
        CLazyBuffer& buffer);

private:
    const GlobalConfig* m_pConfig;     // Not owned
    CForwardList** m_pValues;      // Field Values, Owned
    CStringMap m_Extensions;
    CLazyBuffer& m_Buffer;

public:
    static const int INVALID_FIELD_ID = -1;

    DISALLOW_COPY_CONSTRUCTOR(CHeaderField);
    DISALLOW_ASSIGN_OPERATOR(CHeaderField);
    DISALLOW_DEFAULT_CONSTRUCTOR(CHeaderField);
};

#endif