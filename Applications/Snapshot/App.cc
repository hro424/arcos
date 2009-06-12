/*
 *
 *  Copyright (C) 2008, Waseda University.
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///
/// @brief  Test program for snapshotting and recovery
/// @file   Applications/Snapshot/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  January 2008
///

// $Id: App.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/console.h>
#include <arc/memory.h>
#include <arc/string.h>
#include <arc/types.h>
#include <stdio.h>
#include "App.h"

static int counter __attribute__ ((aligned (4096)));

// Persistent memory
//extern addr_t __persistent_memory_base;

//static int *fi_counter;

static void
InitializeFJ(int count)
{
//    fi_counter = (int *)__persistent_memory_base;
//    *fi_counter = count;
}

static void
InjectFault()
{
    //if (fi_counter > 0) {
    //    *fi_counter--;
        printf(TXT_BRIGHT TXT_FG_RED "FAULT INJECTION\n" TXT_NORMAL);
        *(volatile unsigned long *)0 = 0xDEADBEEF;
    //}
}

///
/// Stack test 1: Nested function calls
///
static void TestStack2();
static void TestStack5();

static void
TestStack1()
{
    ENTER;
    char buffer[SBUFSZ];

    CHECKPOINT();

    memset(buffer, 0x1, SBUFSZ);
    Weight();
    TestStack2();

    EXIT;
}

static void
TestStack2()
{
    ENTER;
    char buffer[SBUFSZ];
    CHECKPOINT();

    memset(buffer, 0x2, SBUFSZ);
    Weight();
    TestStack3();
    EXIT;
}

void
TestStack4()
{
    ENTER;
    char buffer[SBUFSZ];
    CHECKPOINT();

    memset(buffer, 0x4, SBUFSZ);
    //Weight();
    TestStack5();

    EXIT;
}

static void
TestStack5()
{
    ENTER;
    char buffer[SBUFSZ];
    memset(buffer, 0x5, SBUFSZ);
    Weight();
    EXIT;
}

//
// Stack test 2: Recursive call
//

static void
TestStackRecursive(int nest)
{
    ENTER;
    char buffer[SBUFSZ];

    CHECKPOINT();
    printf("level: %d\n", nest);
    memset(buffer, nest, SBUFSZ);
    if (nest > 0) {
        TestStackRecursive(nest - 1);
    }
    EXIT;
}

//
// Data test
//
static void
TestData1()
{
    CHECKPOINT();
    ENTER;

    for (int i = 0; i < 10; i++) {
        printf("local counter: %d\n", i);
        CHECKPOINT();
    }
}

static void
TestData2()
{
    CHECKPOINT();
    ENTER;

    for (int i = 0; i < 10; i++) {
        printf("global counter: %d\n", counter);
        counter++;
        CHECKPOINT();
    }
}

static void
TestHeap1()
{
    InitializeFJ(1);
    CHECKPOINT();
    ENTER;
    char *ptr = static_cast<char *>(malloc(4096));
    InjectFault();
    mfree(ptr);
}

int main(int argc, char *argv[])
{
    // Initialization
    printf("program was initialized\n");

    CHECKPOINT();

    // Initialize the fault injector
    InitializeFJ(4);
    TestStack1();
    InjectFault();

    /*
    InitializeFJ(4);
    TestStackRecursive(4);
    InjectFault();

    InitializeFJ(10);
    TestData1();
    InjectFault();

    InitializeFJ(10);
    TestData2();
    InjectFault();

    TestHeap1();
    */

    printf("End of snapshot test\n");
    return 0;
}

