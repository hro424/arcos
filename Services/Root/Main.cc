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
/// @brief  The entry point of the Arc root task (privileged task)
///
/// @file   Services/Root/Main.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: Main.cc 394 2008-09-02 11:49:53Z hro $

#include <Debug.h>
#include <DebugStream.h>
#include <Ipc.h>
#include <Protocol.h>
#include <System.h>
#include <String.h>
#include <Types.h>
#include <sys/Config.h>

#include "Common.h"
#include "NameService.h"
#include "Pager.h"
#include "Task.h"
#include "Thread.h"

#include <l4/kip.h>
#include <l4/misc.h>
#include <l4/schedule.h>
#include <l4/space.h>
#include <l4/thread.h>

#include <l4/kdebug.h>


#define USER_PAGER      "p3"

///
/// Initialize routine for the server 0.
///
extern void InitProcMan();

///
/// Entry point of the server 0.
///
extern void ProcManMain();

///
/// Initialize routine for the server 1.
///
extern Pager *InitRootPager();

///
/// Entry point of the server 1.
///
extern void RootPagerMain();

///
/// Stack for the server 0
///
static char _initStack[PAGE_SIZE] __attribute__ ((aligned (PAGE_SIZE)));

///
/// Stack for the pager thread (server 1)
///
static char _pagerStack[PAGE_SIZE] __attribute__ ((aligned (PAGE_SIZE)));

///
/// Stack for the root name server
///
//static char _nsStack[PAGE_SIZE] __attribute__ ((aligned (PAGE_SIZE)));


///
/// Thread id of the main thread
///
L4_ThreadId_t           RootId;

///
/// Thread id of the server 0
///
L4_ThreadId_t           ProcManId;

///
/// Thread id of the server 1 (root pager)
///
L4_ThreadId_t           RootPagerId;

///
/// Thread id of the root name server
///
L4_ThreadId_t           RootNsId;

///
/// Boot record of P3
///
L4_Boot_SimpleExec_t    P3BootRec;


extern NameService      _ns;


#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL   10
#endif // VERBOSE_LEVEL

SystemHelper            System(VERBOSE_LEVEL, &__dstream);

void
__exit(int stat)
{
}

void*
operator new(size_t s)
{
    void *ptr = malloc(s);
    if (ptr == 0) {
        FATAL("new: out of memory");
    }
    MEM_TRACE;
    return ptr;
}

void
operator delete(void* ptr)
{
    mfree(ptr);
    MEM_TRACE;
}

void*
operator new[](size_t s)
{
    void* ptr = malloc(s);
    if (ptr == 0) {
        FATAL("new[]: out of memory");
    }
    MEM_TRACE;
    return ptr;
}

void
operator delete[](void* ptr)
{
    mfree(ptr);
    MEM_TRACE;
}


#ifdef SYS_DEBUG
static void
DumpSimpleExec(L4_BootRec_t *rec)
{
    System.Print(System.INFO, "'%s'\n", L4_SimpleExec_Cmdline(rec));
    System.Print(System.INFO, "Entry point %.8lX\n",
                 L4_SimpleExec_InitialIP(rec));
    System.Print(System.INFO, "section   p         v         size\n");
    System.Print(System.INFO, "-------  --------  --------  --------\n");
    System.Print(System.INFO, "text     %.8lX  %.8lX  %.8lX\n",
                 L4_SimpleExec_TextPstart(rec), L4_SimpleExec_TextVstart(rec),
                 L4_SimpleExec_TextSize(rec));
    System.Print(System.INFO, "data     %.8lX  %.8lX  %.8lX\n",
                 L4_SimpleExec_DataPstart(rec), L4_SimpleExec_DataVstart(rec),
                 L4_SimpleExec_DataSize(rec));
    System.Print(System.INFO, "bss      %.8lX  %.8lX  %.8lX\n\n",
                 L4_SimpleExec_BssPstart(rec), L4_SimpleExec_BssVstart(rec),
                 L4_SimpleExec_BssSize(rec));
}
#endif // SYS_DEBUG

///
/// Runs the Root Service as a separate thread.
///
/// @param gidOffset    GID offset
/// @param stack        The base of the stack area
/// @param stackSize    The size of the stack area
/// @param ip           The entry point
///
static L4_ThreadId_t
CreateRootThread(L4_Word_t gidOffset, addr_t stack, size_t stackSize,
                 addr_t ip)
{
    L4_KernelInterfacePage_t    *kip;
    L4_ThreadId_t               tid, self;
    L4_Word_t                   utcb;

    if (gidOffset < 3) {
        FATAL("Special GID is prohibited to use.");
        return L4_nilthread;
    }

    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();
    utcb = PhysLayout::ROOT_UTCB_START + L4_UtcbSize(kip) * (gidOffset - 1);
    tid = L4_GlobalId(L4_ThreadIdUserBase(kip) + gidOffset, 1);
    self = L4_Myself();

    // Create the new thread within our address space
    if (!L4_ThreadControl(tid, self, self, L4_Pager(), (void *)utcb)) {
        System.Print(System.ERROR, "Couldn't create thread: %lu\n",
                     L4_ErrorCode());
        FATAL("Art_CreateRootThread");
    }

    // Give the highest priority
    L4_Set_Priority(tid, 0xFF);

    // Immediatly start the thread
    L4_Start_SpIp(tid, stack + stackSize, ip);

    // Wait for the task be ready
    WaitReady(tid);

    return tid;
}

///
/// Looks for the boot record set by GRUB.
///
/// @param query        the name of the module
/// @param type         the type fo the record
/// @param record       the boot record
///
static stat_t
FindBootRecord(const char *query, L4_Word_t type, L4_BootRec_t **record)
{
    L4_BootRec_t    *rec;
    L4_Word_t       entries;
    void            *info;

    info = (L4_BootInfo_t *)L4_BootInfo(L4_GetKernelInterface());
    if (!L4_BootInfo_Valid(info)) {
        return ERR_NOT_FOUND;
    }

    entries = L4_BootInfo_Entries(info);
    rec = L4_BootInfo_FirstEntry(info);
    for (L4_Word_t cur = 0; cur < entries; cur++) {
        if (L4_BootRec_Type(rec) == type) {
            char *cmdline = (char *)0;
            switch (type) {
                case L4_BootInfo_Module:
                    cmdline = L4_Module_Cmdline(rec);
                    break;
                case L4_BootInfo_SimpleExec:
                    cmdline = L4_SimpleExec_Cmdline(rec);
                    break;
            }

            if (strcmp(query, cmdline) == 0) {
                *record = rec;
                return ERR_NONE;
            }
        }
        rec = L4_BootRec_Next(rec);
    }
    return ERR_NOT_FOUND;
}

///
/// Launches the initial services
///
static void
CreateInitialServices(void)
{
    L4_BootInfo_t   *info;
    L4_BootRec_t    *rec = 0;
    L4_Word_t       entries;
    L4_Word_t       cur;

    info = (L4_BootInfo_t *)L4_BootInfo(L4_GetKernelInterface());
    if (!L4_BootInfo_Valid(info)) {
        FATAL("Invalid boot info");
    }

    entries = L4_BootInfo_Entries(info) - 1;

    // Need to find P3 in advance
    if (FindBootRecord(USER_PAGER, L4_BootInfo_SimpleExec, &rec) != ERR_NONE) {
        System.Print("p3 not found");
        FATAL("");
    }

#ifdef SYS_DEBUG
    DumpSimpleExec(rec);
#endif // SYS_DEBUG

    memcpy(&P3BootRec, rec, sizeof(L4_Boot_SimpleExec_t));

    rec = L4_BootInfo_FirstEntry(info);

    // Assume that the first two entries are sigma0 and roottask
    for (cur = 0; cur < 2; cur++, rec = L4_BootRec_Next(rec));

    //for each relocatable entry in the module list
    for (; cur < entries; cur++, rec = L4_BootRec_Next(rec)) {
        if (L4_BootRec_Type(rec) == L4_BootInfo_SimpleExec) {
            if (strncmp(L4_SimpleExec_Cmdline(rec), USER_PAGER, 3) == 0) {
                continue;
            }

            //
            // Run P3 with the module name and address
            //
#ifdef SYS_DEBUG
            DumpSimpleExec(rec);
#endif // SYS_DEBUG
            if (ExecInitialTask(rec) != ERR_NONE) {
                FATAL("Failed to create an initial process");
            }
        }
        else if (L4_BootRec_Type(rec) == L4_BootInfo_Module) {
            char* ptr = reinterpret_cast<char*>(L4_Module_Start(rec));
            if (strncmp(ptr, "!<arch>\n", 8) == 0) {
                if (ExecInitialTaskPm(rec) != ERR_NONE) {
                    FATAL("Failed to create an initial process");
                }
                continue;
            }

            if (strcmp(L4_Module_Cmdline(rec), "ramdisk") == 0) {
                L4_ThreadId_t reg;
                reg.raw = L4_Module_Start(rec);
                _ns.Insert("ramdisk_start", reg);
                reg.raw = L4_Module_Size(rec);
                _ns.Insert("ramdisk_size", reg);
                continue;
            }
        }
    }
}


int
main()
{
    RootId = L4_Myself();

    DOUT("pager stack @ %p\n", _pagerStack);
    DOUT("init stack @ %p\n", _initStack);

    //
    // Initialize the physical memory
    //
    InitRootPager();

    //
    // Spawn the root pager thread
    //
    RootPagerId = CreateRootThread(Thread::TID_PAGER_OFFSET,
                                   reinterpret_cast<addr_t>(_pagerStack),
                                   PAGE_SIZE,
                                   reinterpret_cast<addr_t>(RootPagerMain));

    InitProcMan();

    //
    // Spawn the process manager thread
    //
    //stack = Pager.Allocate();
    ProcManId = CreateRootThread(Thread::TID_INIT_OFFSET, 
                                 reinterpret_cast<addr_t>(_initStack),
                                 PAGE_SIZE,
                                 reinterpret_cast<addr_t>(ProcManMain));

    //stack = Pager.Allocate();
    //RootNsId = CreateRootThread(Thread::TID_NS_OFFSET, stack, PAGE_SIZE,
    //                            reinterpret_cast<addr_t>(RootNsMain));

    //
    // Run the intial services
    //
    CreateInitialServices();

    //L4_Set_Priority(L4_Myself(), 0);

    DOUT("ART main thread is going to sleep ...\n");

    L4_Sleep(L4_Never);

    FATAL("Never reach here");

    return ERR_NONE;
}

