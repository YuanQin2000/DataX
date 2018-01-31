/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TEST_OBJECT_H__
#define __TEST_OBJECT_H__

#include <stdio.h>

class CTestObject
{
public:
    CTestObject(const char* pName = "DEFAULT") : m_pName(pName) {}
    virtual ~CTestObject() {}

    const char* Name() const { return m_pName; }
    void Print() const { printf("[%s]", m_pName); }

private:
    const char* m_pName;
};

#endif