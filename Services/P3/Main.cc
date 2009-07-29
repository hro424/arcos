/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
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
/// @brief  Pager and loader starting point
/// @file   Services/P3/Main.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Main.cc 426 2008-10-01 09:38:07Z hro $

#include <DebugStream.h>
#include <Ipc.h>
#include <MemoryAllocator.h>
#include <String.h>
#include <Types.h>
#include <Random.h>

#include "IpcHandler.h"
#include "Loader.h"
#include "Pel.h"
#include "Task.h"

#include <System.h>

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL   0
#endif // VERBOSE_LEVEL

SystemHelper System(VERBOSE_LEVEL, &__dstream);

static const char*
get_base_name(const char* name)
{
    char*   ptr = (char*)name;
    char*   base = ptr;

    while (*ptr != 0) {
        if (*ptr == '/') {
            base = ptr + 1;
        }
        ptr++;
    }

    return base;
}

static stat_t
Register(const char *name, L4_ThreadId_t tid)
{
    L4_Msg_t    msg;
    L4_Word_t   len;
    L4_Word_t*  reg;
    stat_t      err;
    ENTER;

    const char* base_name = get_base_name(name);
    len = (strlen(base_name) + 1 + sizeof(L4_Word_t) - 1) / sizeof(L4_Word_t);
    if (len >= __L4_NUM_MRS - 1) {
        len = __L4_NUM_MRS - 2;
    }

    reg = (L4_Word_t *)malloc(sizeof(L4_Word_t) * (len + 1));
    reg[0] = tid.raw;
    memcpy(&reg[1], base_name, strlen(name) + 1);
    L4_Put(&msg, MSG_NS_INSERT, len + 1, reg, 0, 0);
    err = Ipc::Call(Pel::RootTask(), &msg, &msg);
    mfree(reg);

    EXIT;
    return err;
}

static stat_t
Deregister(const char* name)
{
    L4_Msg_t    msg;
    L4_Word_t   len;
    L4_Word_t*  reg;
    stat_t      err;

    const char* base_name = get_base_name(name);
    System.Print("deregister '%s'\n", base_name);
    len = (strlen(base_name) + 1 + sizeof(L4_Word_t) - 1) / sizeof(L4_Word_t);
    if (len >= __L4_NUM_MRS - 1) {
        len = __L4_NUM_MRS - 2;
    }

    reg = (L4_Word_t *)malloc(sizeof(L4_Word_t) * len);
    memcpy(reg, base_name, strlen(name) + 1);
    L4_Put(&msg, MSG_NS_REMOVE, len, reg, 0, 0);
    err = Ipc::Call(Pel::RootTask(), &msg, &msg);
    mfree(reg);

    return err;
}

static inline void
ReadTSC(unsigned long* hi, unsigned long* lo)
{
    asm volatile ("rdtsc" : "=a" (*lo), "=d" (*hi));
}


int
main(int argc, char* argv[])
{
#ifdef TEST_PF_COST
    EvaluatePF();
#endif // TEST_PF_COST

    static Task         task;
    static TaskLoader   loader;
    static IpcHandler   handler;
    stat_t              stat;
    Int                 task_state = 0;
    Int                 type;
    Int                 freq;
    UInt                hi;
    UInt                lo;

    if (argc < 1) {
        return ERR_INVALID_ARGUMENTS;
    }

    ReadTSC(&hi, &lo);
    srand(lo);

    if (strncmp(argv[0], "fexec", 6) == 0) {
        if (argc < 4) {
            System.Print(System.ERROR, "Invalid arguments\n");
            return ERR_INVALID_ARGUMENTS;
        }

        type = atoi(argv[1]);
        freq = atoi(argv[2]);
        argc -= 3;
        argv += 3;
    }
    else {
        type = 0;
        freq = 0;
    }

    char* fs = argv[0];
    char* path = argv[0];
    for (size_t i = 0; i < strlen(argv[0]); i++) {
        if (path[i] == ':') {
            path[i] = '\0';
            path = &path[i + 1];
            argv[0] = path;
        }
    }

restart:
    // Create and initialize a new address space
    task.Initialize(task_state);

    // Load a program to the address space
    if (loader.Load(&task, fs, path, type, freq) != ERR_NONE) {
        goto exit;
    }

    if (task_state == 0) {
        Register(path, task.main_tid);
    }
    if (task_state == 1) {
        task.user_state = Task::USER_READY;
    }

    DOUT("Start '%s'\n", path);
    // Run the main thread of the address space
    task.Start(argc, argv, task_state);

    ReadTSC(&hi, &lo);
    System.Print("P3(%.8lX): Start (time %lu:%lu)\n", L4_Myself().raw, hi, lo);
    stat = handler.HandleIpc(&task);
    ReadTSC(&hi, &lo);
    System.Print("P3(%.8lX): Exit (time %lu:%lu)\n", L4_Myself().raw, hi, lo);

    if (stat != ERR_NONE || handler.UserExitCode() == 2) {
        if (task.user_state == Task::USER_READY) {
            task_state = 1;
        }
        else {
            task_state = 2;
        }
        type = 0;   // Invalidate fault injection
        task.Unmap();
        System.Print("Restart '%s'\n", path);
        goto restart;
    }

    Deregister(argv[0]);

exit:
    task.Delete();
    System.Print("P3 %.8lX exit. Bye!\n", L4_Myself().raw);
    return 0;
}

