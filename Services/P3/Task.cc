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
/// @breif  User task representation
/// @file   Services/P3/Task.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Task.cc 426 2008-10-01 09:38:07Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>
#include "Pel.h"
#include "Segment.h"
#include "Task.h"

Task::Task() : pm_initialized(FALSE)
{
}

stat_t
Task::Initialize(Int state)
{
    L4_Msg_t    msg;
    L4_Word_t   reg;
    stat_t    err;
    ENTER;

    if (state == 0) {
        reg = L4_Myself().raw;
        L4_Put(&msg, MSG_ROOT_NEW_TSK, 1, &reg, 0, 0);

        err = Ipc::Call(Pel::RootTask(), &msg, &msg);
        if (err != ERR_NONE) {
            return err;
        }
        main_tid.raw = L4_Get(&msg, 0);
    }

    segment[0] = &head;
    segment[NSEGMENTS - 1] = &tail;
    segment_counter = 1;

    head.Initialize();
    tail.Initialize();

    user_state = USER_INIT;

    EXIT;
    return ERR_NONE;
}

void
Task::AddSegment(Segment* seg)
{
    if (segment_counter < NSEGMENTS - 1) {
        segment[segment_counter] = seg;
        segment_counter++;
    }
}

Segment *
Task::GetSegment(addr_t address)
{
    ENTER;
    for (int i = 0; i < NSEGMENTS; i++) {
        if (segment[i] != 0 && segment[i]->Hit(address)) {
            EXIT;
            return segment[i];
        }
    }
    return 0;
}

void
Task::InitText(addr_t ent, addr_t base, size_t size)
{
    ENTER;
    DOUT("base %.8lX, size %u\n", base, size);
    this->entry = ent;
    text.Initialize(base, base + size);
    AddSegment(&text);
    EXIT;
}

void
Task::InitData(addr_t base, size_t size)
{
    ENTER;
    DOUT("base %.8lX, size %u\n", base, size);
    data.Initialize(base, base + size);
    AddSegment(&data);
    EXIT;
}

void
Task::InitBss(addr_t base, size_t size)
{
    ENTER;
    DOUT("base %.8lX, size %u\n", base, size);
    FATAL("not implemented\n");
    EXIT;
}

void
Task::InitPm(addr_t base, size_t size)
{
    ENTER;
    DOUT("base %.8lX, size %u\n", base, size);
    if (size > 0) {
        if (!pm_initialized) {
            if (base == 0) {
                base = PAGE_ALIGN(data.EndAddress());
            }
            pm.Initialize(base, PAGE_ALIGN(base + size));
            pm_initialized = TRUE;
        }
        AddSegment(&pm);
    }
    else {
        base = PAGE_ALIGN(data.EndAddress());
        pm.Initialize(base, base);
    }
    EXIT;
}

void
Task::InitMemory()
{
    ENTER;
    // Make a barrier between PM and the heap
    heap.Initialize(pm.EndAddress() + PAGE_SIZE);
    AddSegment(&heap);
    stack.Initialize(VirtLayout::USER_STACK_END);
    AddSegment(&stack);
    shm.Initialize(VirtLayout::SHM_START,
                   VirtLayout::USER_TEXT_START);
    AddSegment(&shm);
    EXIT;
}

void
Task::Unmap()
{
    ENTER;
    text.Release();
    data.Release();
    heap.Release();
    stack.Release();
    EXIT;
}

stat_t
Task::Delete()
{
    L4_Msg_t    msg;
    L4_Word_t   reg;
    ENTER;

    reg = main_tid.raw;
    L4_Put(&msg, MSG_ROOT_DEL_TSK, 1, &reg, 0, 0);

    EXIT;
    return Ipc::Call(Pel::RootTask(), &msg, &msg);
}

stat_t
Task::Start(Int argc, char* argv[], Int state)
{
    L4_Word_t   reg[2];
    L4_Msg_t    msg;
    stat_t      err;
    ENTER;

    reg[1] = stack.SetupArgs(heap.StartAddress(), argc, argv, state);
    reg[0] = entry;

    DOUT("start task(%.8lX): ip %.8lX sp %.8lX heap %.8lX state %u\n",
         main_tid.raw, reg[0], reg[1], heap.StartAddress(), state);
    if (state == 0) {
        // Activate an inactive task
        L4_Put(&msg, 0, 2, reg, 0, 0);
        err = Ipc::Send(main_tid, &msg);
    }
    else {
        // Restart a task
        L4_Word_t       dummy;
        L4_ThreadId_t   dummy_tid;
        // Cancel IP, SP, and receiving IPC
        L4_Word_t       control = (1 << 4) | (1 << 3) | (1 << 1);

        L4_ExchangeRegisters(main_tid, control, reg[1], reg[0], 0, 0,
                             L4_nilthread,
                             &dummy, &dummy, &dummy, &dummy, &dummy,
                             &dummy_tid);
        err = ERR_NONE;
    }

    EXIT;
    return err;
}

