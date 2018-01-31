/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __CONFIG_OBJECT_H__
#define __CONFIG_OBJECT_H__

#include "Common/Tree.h"
#include "XML/XMLData.h"

typedef CTree<CXMLData::XMLElement> tConfigRoot;
typedef CTree<CXMLData::XMLElement>::CNode tConfigNode;

class IConfigObject
{
public:
    virtual bool Initialize(tConfigNode* pConfigData) = 0;
};

#endif