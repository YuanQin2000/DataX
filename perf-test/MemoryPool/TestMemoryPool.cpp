/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <cstdio>
#include <cstdlib>

#include "TestMemoryPoolObject.h"

int main(int argc, char* argv[])
{
    int size;
    if (argc != 2) {
        std::printf("Usage: %s <size>\n", argv[0]);
        std::exit(1);
    }

    size = atoi(argv[1]);
    TObject** tp = new TObject*[size];
    TObject1** tp1 = new TObject1*[size];
    TObject2** tp2 = new TObject2*[size];

    for (int i = 0; i < size; i++) {
        tp[i] = new TObject;
        tp[i]->Set(i, i + 1);
        tp1[i] = new TObject1;
        tp1[i]->Set(i);
        tp2[i] = new TObject2;
        tp1[i]->Set(i);
    }

#if 1
    for (int i = 0; i < size; i++) {
        delete tp[i];
    }
    for (int i = 0; i < size; i++) {
        delete tp1[i];
    }
    for (int i = 0; i < size; i++) {
        delete tp2[i];
    }

    delete [] tp;
    delete [] tp1;
    delete [] tp2;
#endif
    return 0;
}

