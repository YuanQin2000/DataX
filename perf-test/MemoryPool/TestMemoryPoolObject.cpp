/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <cstdio>

#include "TestMemoryPoolObject.h"

/*
 * Implementation for TObject
 */
#ifdef __USE_MEMORY_POOL__
CMemoryPool<TObject>* TObject::mPool = CMemoryPool<TObject>::CreateInstance(NULL);

void* TObject::operator new(size_t size)
{
//    std::printf("TObject(size=%d)::operator new(%d)\n", sizeof(TObject), size);
    return mPool->AllocateCell();
}

void TObject::operator delete(void* p)
{
//    std::printf("TObject::operator delete(%p)\n", p);
    mPool->ReleaseCell(p);
}
#endif


TObject::TObject(int a, int b) : mA(a), mB(b)
{
//    std::printf("TObject::TObject\n");
}

TObject::~TObject()
{
}

const char* TObject::Name() const
{
    return "TObject";
}


/*
 * Implementation for TObject1
 */
#ifdef __USE_MEMORY_POOL__
CMemoryPool<TObject1>* TObject1::mPool = CMemoryPool<TObject1>::CreateInstance(NULL);

void* TObject1::operator new(size_t size)
{
//    std::printf("TObject1(size=%d)::operator new(%d)\n", sizeof(TObject1), size);
    return mPool->AllocateCell();
}

void TObject1::operator delete(void* p)
{
//    std::printf("TObject1::operator delete(%p)\n", p);
    mPool->ReleaseCell(p);
}
#endif

TObject1::TObject1(int a) : mA(a)
{
//    std::printf("TObject1::TObject1\n");
}


/*
 * Implementation for TObject2
 */
#ifdef __USE_MEMORY_POOL__
CMemoryPool<TObject2>* TObject2::mPool = CMemoryPool<TObject2>::CreateInstance(NULL);

void* TObject2::operator new(size_t size)
{
//    std::printf("TObject2(size=%d)::operator new(%d)\n", sizeof(TObject2), size);
    return mPool->AllocateCell();
}

void TObject2::operator delete(void* p)
{
//    std::printf("TObject2::operator delete(%p)\n", p);
    mPool->ReleaseCell(p);
}
#endif

TObject2::TObject2(int c) : TObject(), mC(c)
{
//    std::printf("TObject2::TObject2\n");
}

TObject2::~TObject2()
{
}


const char* TObject2::Name() const
{
    return "TObject2";
}

void TObject2::print() const
{
//    std::printf("TObject2(%d)::print\n", mC);
}
