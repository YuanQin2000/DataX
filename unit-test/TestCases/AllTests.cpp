/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "CppUTest/CommandLineTestRunner.h"

/**
 * CppUTest manual page: https://cpputest.github.io/manual.html
 */
int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}

#include "AllTests.h"