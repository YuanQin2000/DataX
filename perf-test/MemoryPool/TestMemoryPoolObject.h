/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <new>
#include "Memory/MemoryPool.h"

class TObject
{
public:
    TObject(int a = 0, int b = 0);
    virtual ~TObject();

    virtual const char* Name() const;
    void Set(int a, int b) { mA = a, mB = b; }

private:
    int mA;
    int mB;

#ifdef __USE_MEMORY_POOL__
public:
    void* operator new(size_t size);
    void operator delete(void* p);

private:
    static CMemoryPool<TObject>* mPool;
#endif
};

class TObject1
{
public:
    TObject1(int a = 0);

    void Set(int a) { mA = a; }

private:
    int mA;

#ifdef __USE_MEMORY_POOL__
public:
    void* operator new(size_t size);
    void operator delete(void* p);

private:
    static CMemoryPool<TObject1>* mPool;
#endif
};

class TObject2 : public TObject
{
public:
    TObject2(int c = 0);
    ~TObject2();

    void Set(int a) { mC = a; }
    const char* Name() const;
    void print() const;

private:
    int mC;

#ifdef __USE_MEMORY_POOL__
public:
    void* operator new(size_t size);
    void operator delete(void* p);

private:
    static CMemoryPool<TObject2>* mPool;
#endif
};

#endif
