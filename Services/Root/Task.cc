/*
 *
 *  Copyright (C) 2006, 2007, 2008, Waseda University.
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
/// @brief  Operations for tasks
///
/// @file   Services/Root/CoreService.c
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @since  2006
///

// $Id: Task.cc 394 2008-09-02 11:49:53Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <List.h>
#include <Protocol.h>
#include <System.h>
#include <Types.h>
#include <LoadInfo.h>
#include <TaskMap.h>
#include <sys/Config.h>

#include "Common.h"
#include "PageAllocator.h"
#include "Pager.h"
#include "PageFrame.h"
#include "PageFrameTable.h"
#include "Space.h"
#include "Task.h"
#include "Thread.h"

#include <l4/thread.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/schedule.h>
#include <l4/kip.h>

extern L4_ThreadId_t        ProcManId;
extern L4_ThreadId_t        RootPagerId;
extern L4_Boot_SimpleExec_t P3BootRec;

///
/// The head of the address space list which is used by FindTask
///
static Space            *_space_list;

///
/// Mutex for the address space list
///
static Mutex            _mutex;

///
/// The fpage object that indicates the KIP area
///
static L4_Fpage_t       _kipArea;

///
/// The fpage object that indicates the user TCB area where multiple
/// UTCBs reside
///
static L4_Fpage_t       _utcbArea;

///
/// The size of a user TCB
///
static L4_Word_t        _utcbSize;

///
/// Setup the KIP and UTCB areas
///
void
InitializeSpaceInfo()
{
    L4_KernelInterfacePage_t*   kip;

    kip = (L4_KernelInterfacePage_t*)L4_GetKernelInterface();

    _kipArea = L4_Fpage(VirtLayout::KIP_START, L4_KipAreaSize(kip));
    _utcbSize = L4_UtcbSize(kip);
    _utcbArea = L4_Fpage(VirtLayout::KIP_START + (L4_UtcbSize(kip) *
                                                  Config::MAX_LOCAL_THREADS),
                         _utcbSize * Config::MAX_LOCAL_THREADS);

    DOUT("KIP area @ %.8lX (%lu bytes)\n",
         L4_Address(_kipArea), L4_Size(_kipArea));
    DOUT("UTCB area @ %.8lX (%lu bytes)\n",
         L4_Address(_utcbArea), L4_Size(_utcbArea));
}

///
/// Registers the specified address space object to the list.  The list
/// is used by FindTask.
///
/// @param s    the address space object
///
static inline void
Register(Space* s)
{
    _mutex.Lock();
    s->next = _space_list;
    _space_list = s;
    _mutex.Unlock();
}

///
/// Removes the specified address space object from the list.
///
/// @param s    the address space object to be removed
///
static inline void
Deregister(Space* s)
{
    _mutex.Lock();
    Space **pp = &_space_list;
    while (*pp != 0 && *pp != s) {
        pp = &(*pp)->next;
    }
    if (*pp != 0) {
        *pp = s->next;
        s->next = 0;
    }
    _mutex.Unlock();
}

///
/// Allocates an address space object.
///
static Space*
CreateSpace(Bool shadow)
{
    Space* s = new Space(_kipArea, _utcbArea, _utcbSize, shadow);
    Register(s);
    return s;
}

///
/// Releases the specified address space object.
///
/// @param s    the address space object
///
static void
DeleteSpace(Space* s)
{
    Deregister(s);
    delete s;
}

//--------------------------------------------------------------------------
//      Global
//--------------------------------------------------------------------------

///
/// Ask the pager of the thread to start it using the given instruction and
/// stack pointers.
///
stat_t
StartThread(const Thread *thread, L4_ThreadId_t pager,
            L4_Word_t ip, L4_Word_t sp)
{
    L4_Msg_t	msg;
    L4_Word_t	reg[3];

    DOUT("EXEC ip %.8lX sp %.8lX\n", ip, sp);
    reg[0] = thread->Id.raw;
    reg[1] = ip;
    reg[2] = sp;

    L4_Put(&msg, MSG_PAGER_TH, 3, reg, 0, (void *)0);
    return Ipc::Call(pager, &msg, &msg);
}

/**
 * Create a new task (an address space + a root thread).
 */
stat_t
CreateTask(Space **s, Bool shadow)
{
    stat_t      err = ERR_UNKNOWN;
    L4_Word_t   dummy;
    Space*      space;
    ENTER;

    space = CreateSpace(shadow);
    if (space == 0) {
        return ERR_OUT_OF_MEMORY;
    }

#ifdef NICTA_EMBEDDED
    if (!L4_ThreadControl(space->GetRootThread()->Id,
                          space->GetRootThread()->Id,
                          L4_Myself(), L4_nilthread, L4_anythread,
                          L4_anythread, (void *)-1)) {
#else
    if (!L4_ThreadControl(space->GetRootThread()->Id,
                          space->GetRootThread()->Id,
                          L4_Myself(), L4_nilthread,
                          (void *)space->GetRootThread()->Utcb)) {
#endif
        DeleteSpace(space);
        err = (stat_t)L4_ErrorCode();
        System.Print(System.ERROR, "Couldn't create thread: %lu\n",
                     L4_ErrorCode());
        goto exit;
    }

    //
    // Configure the space
    //
#ifdef NICTA_EMBEDDED
    if (L4_SpaceControl(space->GetRootThread->Id, 0, space->KipArea(),
                        space->UtcbArea(), &dummy)) {
#else
    if (L4_SpaceControl(space->GetRootThread()->Id, 0, space->KipArea(),
                        space->UtcbArea(), L4_nilthread, &dummy)) {
#endif
        *s = space;
        err = ERR_NONE;
    }
    else {
        L4_ThreadControl(space->GetRootThread()->Id, L4_nilthread, L4_nilthread,
                         L4_nilthread, (void *)-1);

        DeleteSpace(space);
        err = (stat_t)L4_ErrorCode();
        System.Print(System.ERROR, "Couldn't create space: %lu\n",
                     L4_ErrorCode());
    }

exit:
    EXIT;
    return err;
}

stat_t
DeleteTask(Space* space)
{
    ENTER;

    // Terminate and delete all the resident threads
    List<Thread*>& list = space->GetResidents();
    Iterator<Thread*>& it = list.GetIterator();
    while (it.HasNext()) {
        Thread* th = it.Next();
        TerminateThread(th);
    }

    // Delete the space
    DeleteSpace(space);

    EXIT;
    return ERR_NONE;
}

stat_t
FindTask(L4_ThreadId_t tid, Space **obj)
{
    Thread*     thread;
    stat_t      err = ERR_NOT_FOUND;
    ENTER;

    _mutex.Lock();

    Space *cur = _space_list;
    while (cur != 0) {
        if (cur->FindThread(tid, &thread) == ERR_NONE) {
            *obj = cur;
            err = ERR_NONE;
            break;
        }
        cur = cur->next;
    }

    _mutex.Unlock();

    EXIT;
    return err;
}


stat_t
CreateThread(Space *space, Thread **th)
{
    stat_t  err;
    Thread* thread;
    ENTER;

    err = space->CreateThreadObj(&thread);
    if (err != ERR_NONE) {
        return err;
    }

    //
    // Thread creation
    // (cond: space specifier != nilthread && dest not existing)
    //
#ifdef NICTA_EMBEDDED
    if (L4_ThreadControl(thread->Id, space->GetRootThread()->Id, L4_Myself(),
                         L4_nilthread, L4_anythread, L4_anythread,
                         (void *)-1)) {
#else
    if (L4_ThreadControl(thread->Id, space->GetRootThread()->Id, L4_Myself(),
                         L4_nilthread, (void *)-1)) {
#endif
        *th = thread;
        err = ERR_NONE;
    }
    else {
        System.Print(System.ERROR, "Couldn't create thread: %lu\n",
                     L4_ErrorCode());
        space->DeleteThreadObj(thread);
        err = (stat_t)L4_ErrorCode();
    }
    
    EXIT;
    return err;
}

stat_t
TerminateThread(Thread* thread)
{
    stat_t              err;
    L4_ThreadState_t    stat;
    ENTER;

    if (thread->Irq != 0) {
        L4_DeassociateInterrupt(L4_GlobalId(thread->Irq, 1));
    }

    stat = L4_AbortIpc_and_stop(thread->Id);
    //System.Print("aborted thread %.8lX state %lu\n",
    //             thread->Id.raw, stat.raw);

    if (L4_ThreadControl(thread->Id, L4_nilthread, L4_nilthread,
                         L4_nilthread, (void *)-1)) {
        err = ERR_NONE;
    }
    else {
        err = (stat_t)L4_ErrorCode();
    }

    // Deletes the thread object anyway.
    //thread->AddressSpace->DeleteThreadObj(thread);

    EXIT;
    return err;
}

///
/// Turn a thread into active state, i.e. assign it a scheduler and process,
/// which turns it into active state for L4.
///
stat_t
ActivateThread(const Thread* thread, L4_ThreadId_t scheduler,
               L4_ThreadId_t pager)
{
    // Thread modification (space specifier != nilthread, dest exists)
    DOUT("tid: %.8lX, space: %.8lX, sched: %.8lX, pager: %.8lX, utcb: %.8lX\n",
         thread->Id.raw, thread->AddressSpace->GetRootThread()->Id.raw,
         scheduler.raw, pager.raw, thread->Utcb);
    if (!L4_ThreadControl(thread->Id,
                          thread->AddressSpace->GetRootThread()->Id,
                          scheduler, pager, (void *)thread->Utcb)) {
        System.Print(System.ERROR, "Couldn't activate thread: %lu\n",
                     L4_ErrorCode());
    }

    return ERR_NONE;
}

//
// Allocates a page for p3's initial stack
//
stat_t
AllocateStack(Space *space, L4_Word_t *phys)
{
    PageFrame   *frame;
    ENTER;

    MainPa.Allocate(1, &frame);

    frame->SetOwner(space->GetRootThread()->Id);
    frame->SetOwnerRights(PAGE_PERM_READ_WRITE);

    space->InsertMap((VirtLayout::P3_STACK_END - PAGE_SIZE), frame);

    *phys = MainPft.GetAddress(frame);

    EXIT;
    return ERR_NONE;
}

void
ReleaseStack(Space *space, L4_Word_t phys)
{
    space->RemoveMap(VirtLayout::P3_STACK_END - PAGE_SIZE);
}

void
RegisterInitialExecutable(Space *space, L4_BootRec_t *rec)
{
    L4_Word_t   phys, virt, size;
    PageFrame   *frame;
    ENTER;

    // Register text section
    phys = L4_SimpleExec_TextPstart(rec);
    virt = L4_SimpleExec_TextVstart(rec);
    size = L4_SimpleExec_TextSize(rec);
    for (L4_Word_t i = 0; i < PAGE_ALIGN(size) - 1; i += PAGE_SIZE) {
        // NOTE: Each of the pages where the initial programs are placed is
        // not managed by the page allocator and set its length to 1.
        frame = MainPft.GetFrame(phys + i);
        frame->SetOwnerRights(PAGE_PERM_READ_EXEC);
        space->InsertMap(virt + i, frame);
    }

    // Register data section
    phys = L4_SimpleExec_DataPstart(rec);
    virt = L4_SimpleExec_DataVstart(rec);
    size = L4_SimpleExec_DataSize(rec);
    for (L4_Word_t i = 0; i < PAGE_ALIGN(size) - 1; i += PAGE_SIZE) {
        // NOTE: Each of the pages where the initial programs are placed is
        // not managed by the page allocator and set its length to 1.
        //XXX: Reserve the data section as fully accessible.
        frame = MainPft.GetFrame(phys + i);
        frame->SetOwnerRights(PAGE_PERM_FULL);
        frame->SetAttribute(PAGE_ATTR_COW);
        space->InsertMap(virt + i, frame);
    }

    EXIT;
}

L4_Word_t
RegisterInitialModule(Space *space, L4_BootRec_t *rec)
{
    L4_Word_t   phys, virt, size;
    PageFrame*  frame;

    phys = L4_Module_Start(rec);
    size = L4_Module_Size(rec);
    //virt = PAGE_ALIGN(USER_STACK_END - size);
    //HRO: quick hack
    virt = PAGE_ALIGN(VirtLayout::USER_STACK_END - size) - PAGE_SIZE * 10;
    for (L4_Word_t i = 0; i < PAGE_ALIGN(size) - 1; i += PAGE_SIZE) {
        frame = MainPft.GetFrame(phys + i);
        frame->SetOwnerRights(PAGE_PERM_READ_EXEC);
        space->InsertMap(virt + i, frame);
    }
    return virt;
}

///
/// @param stack_page       the first stack page
/// @param dest             the destination address the arguments set up to
/// @param argc             the source ARGC
/// @param argv             the source ARGV
///

static addr_t
ConvertVirtual(addr_t stack_page, addr_t phys)
{
    return VirtLayout::P3_STACK_END - (stack_page + PAGE_SIZE - phys);
}

static void
CopyArgs(addr_t stack_page, addr_t dest, int argc, char** argv)
{
    L4_Word_t*  args = reinterpret_cast<L4_Word_t*>(dest);

    // Argc
    args[0] = static_cast<L4_Word_t>(argc);
    // Argv
    args[1] = ConvertVirtual(stack_page, reinterpret_cast<addr_t>(&args[2]));

    addr_t*  argv_phys = reinterpret_cast<addr_t*>(&args[2]);
    char*   dest_phys = reinterpret_cast<char*>(&args[2 + argc]);

    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]) + 1;
        memcpy(dest_phys, argv[i], len);
        argv_phys[i] = ConvertVirtual(stack_page,
                                      reinterpret_cast<addr_t>(dest_phys));
        dest_phys += len;
    }
}

///
/// Creates an initial process and its P3.
///
/// @param start    the start address where the process image is placed
/// @param size     the size of the process image in byte
///
stat_t
ExecInitialTask(L4_BootRec_t *task)
{
    stat_t          err;
    Space*          space;
    L4_Word_t*      stack;
    L4_Word_t       stack_size;
    L4_Word_t       stack_page;
    LoadInfo*       load_info;

    ENTER;

    if (L4_Type(task) != L4_BootInfo_SimpleExec) {
        return ERR_INVALID_ARGUMENTS;
    }
    
    //
    // Create a space for the shadow task
    //
    err = CreateTask(&space, TRUE);
    if (err != ERR_NONE) {
        return err;
    }
    
    space->SetPager(RootPagerId);

    //
    // Load programs into the main memory
    //
    RegisterInitialExecutable(space,
                              reinterpret_cast<L4_BootRec_t*>(&P3BootRec));
    RegisterInitialExecutable(space, task);

    //
    // Allocates the stack for the shadow task
    //
    err = AllocateStack(space, &stack_page);
    if (err != ERR_NONE) {
        DeleteTask(space);
        return err;
    }

    DOUT("stack page allocated (phys): %.8lX\n", stack_page);

    //
    // Set up the initial stack for the shadow stack.
    //

    char* command_line = L4_SimpleExec_Cmdline(task);

    stack_size = sizeof(LoadInfo) + sizeof(L4_Word_t) * 5 +
                    strlen(command_line) + 1;
    stack_size = ROUND_UP(stack_size, sizeof(L4_Word_t));

    stack = (L4_Word_t *)(stack_page + PAGE_SIZE - stack_size);
    stack[0] = ProcManId.raw;
    stack[1] = sizeof(LoadInfo);

    load_info = (LoadInfo *)&stack[2];
    load_info->task_map.text_start = L4_SimpleExec_TextVstart(task);
    load_info->task_map.text_size = L4_SimpleExec_TextSize(task);
    load_info->task_map.data_start = L4_SimpleExec_DataVstart(task);
    load_info->task_map.data_size = L4_SimpleExec_DataSize(task);
    load_info->task_map.pm_start = 0;
    load_info->task_map.pm_size = 0;
    load_info->task_map.entry = L4_SimpleExec_InitialIP(task);
    load_info->type = LoadInfo::LOAD_INFO_MEM;

    addr_t dest = reinterpret_cast<addr_t>(&stack[2]) + sizeof(LoadInfo);
    CopyArgs(stack_page, dest, 1, &command_line);

    DOUT("Pel's stack @ %.8lX\n", VirtLayout::P3_STACK_END - stack_size);
#ifdef SYS_DEBUG
    for (size_t i = 0; i < stack_size; i += 4) {
        System.Print("%.8lX %.8lX %.8lX %.8lX\n",
                     stack[i], stack[i + 1], stack[i + 2], stack[i + 3]);
    }
#endif // SYS_DEBUG

    //
    // Run P3
    //
    ActivateThread(space->GetRootThread(), RootId, RootPagerId); 
    err = StartThread(space->GetRootThread(), RootPagerId,
                      L4_SimpleExec_InitialIP(
                                reinterpret_cast<L4_BootRec_t*>(&P3BootRec)),
                      VirtLayout::P3_STACK_END - stack_size);
    if (err != ERR_NONE) {
        System.Print(System.ERROR, "%s\n", stat2msg[err]);
        FATAL("failed to start initial thread");
    }

    EXIT;
    return ERR_NONE;
}

stat_t
ExecInitialTaskPm(L4_BootRec_t *task)
{
    stat_t      err;
    Space*      space;
    L4_Word_t*  stack;
    L4_Word_t   stack_size;
    L4_Word_t   stack_page;
    LoadInfo*   load_info;

    ENTER;

    //
    // Create a space for the shadow task
    //
    err = CreateTask(&space, TRUE);
    if (err != ERR_NONE) {
        return err;
    }
    
    space->SetPager(RootPagerId);

    //
    // Load programs into the main memory
    //
    RegisterInitialExecutable(space,
                              reinterpret_cast<L4_BootRec_t*>(&P3BootRec));
    L4_Word_t start = RegisterInitialModule(space, task);

    //
    // Allocates the stack for the shadow task
    //
    err = AllocateStack(space, &stack_page);
    if (err != ERR_NONE) {
        DeleteTask(space);
        return err;
    }

    DOUT("stack page allocated (phys): %.8lX\n", stack_page);

    //
    // Set up the initial stack for the shadow stack.
    //

    char* command_line = L4_Module_Cmdline(task);

    stack_size = sizeof(LoadInfo) + sizeof(L4_Word_t) * 5; 
    stack_size += strlen(command_line) + 1;
    stack_size = ROUND_UP(stack_size, sizeof(L4_Word_t));
    stack = (L4_Word_t *)(stack_page + PAGE_SIZE - stack_size);
    stack[0] = ProcManId.raw;
    stack[1] = sizeof(LoadInfo);

    load_info = (LoadInfo *)&stack[2];
    load_info->task_map.text_start = start;
    load_info->task_map.text_size = L4_Module_Size(task);
    load_info->task_map.data_start = 0;
    load_info->task_map.data_size = 0;
    load_info->task_map.pm_start = 0;
    load_info->task_map.pm_size = 0;
    load_info->task_map.entry = 0;
    load_info->type = LoadInfo::LOAD_INFO_MODULE;

    addr_t dest = reinterpret_cast<addr_t>(&stack[2]) + sizeof(LoadInfo);
    CopyArgs(stack_page, dest, 1, &command_line);

    //
    // Run P3
    //
    ActivateThread(space->GetRootThread(), RootId, RootPagerId); 
    err = StartThread(space->GetRootThread(), RootPagerId,
                      L4_SimpleExec_InitialIP(
                                reinterpret_cast<L4_BootRec_t*>(&P3BootRec)),
                      VirtLayout::P3_STACK_END - stack_size);
    if (err != ERR_NONE) {
        System.Print(System.ERROR, "%s\n", stat2msg[err]);
        FATAL("failed to start initial thread");
    }

    EXIT;
    return ERR_NONE;
}


stat_t
ExecTask(L4_Word_t argc, char** argv, L4_ThreadId_t *tid)
{
    stat_t          err;
    Space*          space;
    L4_Word_t*      stack;
    L4_Word_t       stack_size;
    L4_Word_t       stack_page;
    LoadInfo*       load_info;
    ENTER;

    //
    // Create a space for P3
    //
    err = CreateTask(&space, TRUE);
    if (ERR_NONE != err) {
        return err;
    }

    space->SetPager(RootPagerId);

    //
    // Load the code into the memory
    //
    RegisterInitialExecutable(space,
                              reinterpret_cast<L4_BootRec_t*>(&P3BootRec));

    //
    // Allocate the stack for P3
    //
    err = AllocateStack(space, &stack_page);
    if (err != ERR_NONE) {
        DeleteTask(space);
        return err;
    }

    //
    // Set up the initial stack for P3.
    //

    stack_size = sizeof(LoadInfo) + sizeof(L4_Word_t) * (4 + argc);

    for (L4_Word_t i = 0; i < argc; i++) {
        stack_size += strlen(argv[i]) + 1;
    }

    stack_size = ROUND_UP(stack_size, sizeof(L4_Word_t));
    stack = (L4_Word_t *)(stack_page + PAGE_SIZE - stack_size);
    stack[0] = ProcManId.raw;
    stack[1] = sizeof(LoadInfo);

    load_info = (LoadInfo *)&stack[2];
    memset(load_info, 0, sizeof(LoadInfo));
    load_info->type = LoadInfo::LOAD_INFO_FILE;

    addr_t dest = reinterpret_cast<addr_t>(&stack[2]) + sizeof(LoadInfo);
    CopyArgs(stack_page, dest, argc, argv);

    //
    // Run P3
    //
    ActivateThread(space->GetRootThread(), RootId, RootPagerId); 
    StartThread(space->GetRootThread(), RootPagerId,
                L4_SimpleExec_InitialIP(
                                reinterpret_cast<L4_BootRec_t*>(&P3BootRec)),
                VirtLayout::P3_STACK_END - stack_size);

    EXIT;
    return ERR_NONE;
}

///
/// Initializes kernel KIP and UTCB areas, threads
/// variables and list of address spaces.
///
void
InitializeTaskManagement()
{
    ENTER;
    InitializeSpaceInfo();
    Space::InitializeTidMap();
    _space_list = 0;
    EXIT;
}

