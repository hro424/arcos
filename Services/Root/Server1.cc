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
/// @brief  The initialization and core routines for the pager
/// @file   Services/Root/Server1.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Server1.cc 429 2008-11-01 02:24:02Z hro $

#include <l4/kdebug.h>
#include <Debug.h>
#include <Ipc.h>
#include <MemoryPool.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>

#include "Common.h"
#include "PageAllocator.h"
#include "PageFrame.h"
#include "PageFrameTable.h"
#include "Pager.h"
#include "Space.h"
#include "Task.h"
#include "Thread.h"

#include <l4/kip.h>
#include <l4/message.h>
#include <l4/types.h>


//-----------------------------------------------------------------------------
//      Pager Main Routines
//-----------------------------------------------------------------------------

///
/// Page frame table for the main memory
///
PageFrameTable          MainPft;

///
/// Page alloctor for the main memory
///
PageAllocator           MainPa;

///
/// Page allocator for the mapped I/O area
///
PageAllocator           IoPa;

///
/// Pager for the main memory
///
Pager                   Pg;

///
/// Pager for the mapped I/O area
///
Pager                   IoPg;

///
/// Heap manager
///
MemoryPool              MemPool;

static PageFrame        *EmptyPage;
static PageFrame        *EmptyCowPage;

///
/// The lenth of the map registers
///
static const L4_Word_t  MAP_REG_LENGTH = 16;

///
/// A temporal storage to pass the mapping items between methods
///
static L4_MapItem_t     _mapregs[MAP_REG_LENGTH];


static stat_t HandleStartThread(L4_Msg_t *msg);
static stat_t HandlePageFault(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandlePageAllocate(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandlePageRelease(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleMapRequest(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleUnmapRequest(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleIoMap(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleIoUnmap(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandlePhys(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleMapBiosPage(L4_ThreadId_t tid, L4_Msg_t *msg);
//static stat_t HandleMapLFB(L4_ThreadId_t tid, L4_Msg_t *msg);

/// Allocate one reserved, unmapped, zeroed memory page and make
/// the global variable EmptyPage point to it.
static void
PrepareEmptyPage()
{
    PageFrame   *frame;
    ENTER;

    MainPa.Allocate(1, &frame);
    frame->SetDestination(~0UL);
    frame->SetType(PAGE_TYPE_RESERVED);
    //frame->SetState(PAGE_STATE_INUSE);
    frame->SetOwner(L4_nilthread);
    frame->SetOwnerRights(PAGE_PERM_READ);
    frame->SetSharer(L4_nilthread);
    frame->SetSharerRights(PAGE_PERM_NONE);
    frame->SetAttribute(PAGE_ATTR_EMPTY | PAGE_ATTR_CONST);
    Pg.ZeroPage(frame);

    EmptyPage = frame;
    EXIT;
}

/// Allocate one reserved, unmapped, zeroed, cow memory page and make
/// the global variable EmptyCowPage point to it.
static void
PrepareEmptyCowPage()
{
    PageFrame   *frame;
    ENTER;

    MainPa.Allocate(1, &frame);
    frame->SetDestination(~0UL);
    frame->SetType(PAGE_TYPE_RESERVED);
    //frame->SetState(PAGE_STATE_INUSE);
    frame->SetOwner(L4_nilthread);
    frame->SetOwnerRights(PAGE_PERM_READ);
    frame->SetSharer(L4_nilthread);
    frame->SetSharerRights(PAGE_PERM_NONE);
    frame->SetAttribute(PAGE_ATTR_EMPTY | PAGE_ATTR_CONST | PAGE_ATTR_COW);
    Pg.ZeroPage(frame);

    EmptyCowPage = frame;
    EXIT;
}

void
InitRootPager()
{
    PageFrameTable* pft;
    L4_Word_t       base;

    ENTER;

    //
    // Create the main page frame table
    //
    CreateMainPft(MainPft);

    //
    // Create the main page allocator
    //
    MainPa.Initialize(&MainPft);

    //System.Print(System.INFO, "Size of page frame: %d\n", sizeof(PageFrame));

    DOUT("Dump Main PFT:\n");
    MainPft.Dump();

    System.Print(System.INFO, "Page allocator initialized\n");

    //
    // Create memory pool and manager
    //
    if (MainPa.Allocate(HEAP_SIZE >> PAGE_BITS, &base) != ERR_NONE) {
        FATAL("Memory pool was not allocated\n");
    }

    MemPool.Initialize(base, base + HEAP_SIZE);
    System.Print(System.INFO,
                 "Memory pool allocated: %.8lX - %.8lX %lu bytes\n",
                 base, base + HEAP_SIZE, HEAP_SIZE);

    //
    // Create the main pager
    //
    Pg.SetPft(&MainPft);

    //
    // Allocate the page table for mapped I/O area
    // 
    pft = CreateIoPft();
    IoPa.Initialize(pft);

    pft->Dump();

    //
    // Create a pager for the mapped I/O area
    //
    IoPg.SetPft(pft);

    PrepareEmptyPage();
    PrepareEmptyCowPage();

    EXIT;
}

///
/// Main pager entry point. It handles starting threads, memory allocation,
/// page faults and mapping, and I/O mapping.
///
void
RootPagerMain()
{
    L4_ThreadId_t   tid;
    L4_MsgTag_t     tag;
    L4_Msg_t        msg;
    stat_t          err = ERR_NONE;

    System.Print(System.INFO, "Starting Root Pager (%.8lX) ... \n",
                 L4_Myself().raw);

    NotifyReady();

begin:
    tag = L4_Wait(&tid);

    for (;;) {
        if (L4_IpcFailed(tag)) {
            switch (Ipc::ErrorCode()) {
                case ERR_IPC_TIMEOUT:
                case ERR_IPC_TIMEOUT_SEND:
                case ERR_IPC_TIMEOUT_RECV:
                case ERR_IPC_NO_PARTNER:
                    goto begin;
                case ERR_IPC_CANCELED:
                case ERR_IPC_ABORTED:
                    goto exit;
                case ERR_IPC_OVERFLOW:
                default:
                    BREAK("ipc error");
                    goto begin;
            }
        }

        L4_MsgStore(tag, &msg);
        switch (L4_Label(tag) & MSG_PAGER_MASK) {
            case MSG_PAGER_TH:
                if ((err = HandleStartThread(&msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:ErrStartThread:%s\n",
                               stat2msg[err]);
                }
                break;
            case MSG_PAGER_PF:
                if ((err = HandlePageFault(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:ErrHandlePF:%s\n",
                               stat2msg[err]);
                }
                break;
            case MSG_PAGER_ALLOCATE:
                if ((err = HandlePageAllocate(tid, &msg)) != ERR_NONE) {
                    //System.Print(System.ERROR, "Srv1:ErrHandlePageAllocate:%s\n",
                    //           stat2msg[err]);
                }
                break;
            case MSG_PAGER_RELEASE:
                if ((err = HandlePageRelease(tid, &msg)) != ERR_NONE) {
                    //System.Print(System.ERROR, "Srv1:ErrHandlePageRelease:%s\n",
                    //           stat2msg[err]);
                }
                break;
            case MSG_PAGER_MAP:
                if ((err = HandleMapRequest(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:ErrHandleMapRequest:%s\n",
                               stat2msg[err]);
                }
                break;
            case MSG_PAGER_UNMAP:
                if ((err = HandleUnmapRequest(tid, &msg)) != ERR_NONE) {
                    //System.Print(System.ERROR, "Srv1:ErrHandleUnmapRequest:%s\n",
                    //           stat2msg[err]);
                }
                break;
            case MSG_PAGER_IOPF:
                BREAK("IOPF");
                break;
            case MSG_PAGER_IOMAP:
                if ((err = HandleIoMap(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:Err_HandleIoMap\n");
                }
                break;
            case MSG_PAGER_IOUNMAP:
                if ((err = HandleIoUnmap(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:Err_HandleIoUnmap\n");
                }
                break;
            case MSG_PAGER_PHYS:
                if ((err = HandlePhys(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:Err_HandlePhys\n");
                }
                break;
            case MSG_PAGER_MAPBIOSPAGE:
                if ((err = HandleMapBiosPage(tid, &msg)) != ERR_NONE) {
                    System.Print(System.ERROR, "Srv1:Err_MapBios\n");
                }
                break;
            default:
                System.Print(System.WARN,
                           "Srv1: Unknown message %lX from %.8lX\n",
                           L4_Label(tag) & MSG_PAGER_MASK, tid.raw);
                break;
        }

        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }
exit:
    FATAL("Pager_Main_ErrNeverReached");
}

///
/// Send the start signal to a thread, along with its SP and IP.
///
static stat_t
HandleStartThread(L4_Msg_t *msg)
{
    L4_ThreadId_t   to;
    L4_MsgTag_t     tag;
    L4_Word_t       reg[2];

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 3)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    to.raw = L4_Get(msg, 0);
    reg[0] = L4_Get(msg, 1);    // instruction pointer
    reg[1] = L4_Get(msg, 2);    // stack pointer

    L4_Put(msg, 0, 2, reg, 0, (void *)0);
    L4_MsgLoad(msg);
    tag = L4_Send(to);
    if (L4_IpcFailed(tag)) {
        return Ipc::ReturnError(msg, (stat_t)L4_ErrorCode());
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandlePageFault(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t       faddr, fip, rwx;
    PageFrame       *frame;
    Space           *space;
    stat_t        status;

    ENTER;
    if (Ipc::CheckPayload(msg, 0, 2)) {
        return ERR_INVALID_ARGUMENTS;
    }

    //
    // Obtains the address space where the thread belongs
    //
    if (FindTask(tid, &space) != ERR_NONE) {
        return ERR_INVALID_SPACE;
    }

    faddr = L4_MsgWord(msg, 0);
    fip = L4_MsgWord(msg, 1);
    rwx = L4_Label(msg) & PAGE_PERM_MASK;

#ifdef SYS_DEBUG
    System.Print("PF @ virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                 faddr, fip, rwx, tid.raw);
#endif // SYS_DEBUG

    faddr &= PAGE_MASK;

    if (!Pg.IsValidAddress(faddr)) {
        return ERR_INVALID_RIGHTS;
    }

    //
    // Search the mapping database of the space to see if there's a page
    // reserved for the fault address.
    //
    // Note: Mapping data base is provided for each address space.  It is
    //       created when an address space is constructed.
    //

    //
    // (1) The fault address is found in the database.
    //
    if (space->SearchMap(faddr, &frame) == ERR_NONE) {
        DOUT("reserved mapping@%.8lX (p:%.8lX)\n",
             faddr, Pg.PhysicalAddress(frame));
        if ((frame->GetOwnerRights() & rwx) == 0) {
            //TODO:
        }

        if (frame->GetPageGroup() != 1) {
            //TODO:
        }

        if (frame->IsShared()) {
            // Shared page must be explicitly mapped via a mapping request.
            // So this frame should be revoked.
            // TODO: count the number of sharers
            space->RemoveMap(faddr);
            MainPa.Release(frame);
            goto anonymous_mapping;
        }

        // Copy on write
        if (frame->IsCOW() && IS_WRITABLE(rwx)) {
            PageFrame   *newf;

            if (MainPa.Allocate(1, &newf) != ERR_NONE) {
                return ERR_OUT_OF_MEMORY;
            }
            DOUT("COW %.8lX (%.8lx -> %.8lX)\n",
                 faddr, Pg.PhysicalAddress(frame), Pg.PhysicalAddress(newf));

            // Remember it's write-accessed.
            frame->AddAccessState(PAGE_STATE_WRITE);

            Pg.CopyPage(newf, frame);
            newf->SetOwner(space->GetRootThread()->Id);
            newf->SetOwnerRights(newf->GetOwnerRights() | PAGE_PERM_WRITE);
            newf->SetSharerRights(PAGE_PERM_NONE);
            newf->SetAttribute(0);

            status = Pg.CreateMapItem(faddr, newf, newf->GetOwnerRights(),
                                      &_mapregs[0]);
            if (status != ERR_NONE) {
                MainPa.Release(newf);
                return status;
            }

            // Update the mapping database
            if (space->SetMap(faddr, newf) == FALSE) {
                FATAL("map DB inconsistency");
            }
        }
        else {
            status = Pg.CreateMapItem(faddr, frame, rwx, &_mapregs[0]);
            if (status != ERR_NONE) {
                return status;
            }
        }
    }
    //
    // (2) Map an anonymous page that is supposed to be assigned to a heap or
    //     a stack area, not any text area.
    //
    else {
anonymous_mapping:
        DOUT("anonymous mapping\n");
        // The text area has to be registered to the mapping database
        if ((fip & PAGE_MASK) == faddr) {
            return ERR_INVALID_RIGHTS;
        }

        if (IS_EXECUTABLE(rwx)) {
            BREAK("anon exec access");
            return ERR_INVALID_RIGHTS;
        }
        else if (IS_WRITABLE(rwx)) {
            // Allocate and map a page
            if (MainPa.Allocate(1, &frame) != ERR_NONE) {
                return ERR_OUT_OF_MEMORY;
            }

            // Reserve 'read-write' permission, but map 'write' permission only
            frame->SetOwner(space->GetRootThread()->Id);
            frame->SetOwnerRights(PAGE_PERM_READ_WRITE);
            frame->SetSharer(L4_nilthread);
            frame->SetSharerRights(PAGE_PERM_NONE);
            frame->SetDestination(faddr);
            frame->AddAccessState(PAGE_STATE_WRITE | PAGE_STATE_READ);
            DOUT("page state: %X\n",
                 frame->GetState() | frame->GetAccessState());

            //XXX
            status = Pg.CreateMapItem(faddr, frame, PAGE_PERM_READ_WRITE,
                                      &_mapregs[0]);
            if (status != ERR_NONE) {
                MainPa.Release(frame);
                return status;
            }
        }
        else if (IS_READABLE(rwx)) {
            status = Pg.CreateMapItem(faddr, EmptyCowPage, PAGE_PERM_READ,
                                      &_mapregs[0]);
            if (status != ERR_NONE) {
                return status;
            }
        }
        else {
            return ERR_NONE;
        }

        DOUT("page state: %X\n", frame->GetState() | frame->GetAccessState());
        // NOTE: The fault address is referenced in the 1st branch (1) at the
        // next time as it is registered.
        space->InsertMap(faddr, frame);
    }

    L4_Clear(msg);
    L4_Put(msg, 0, 0, (L4_Word_t *)0, 2, &_mapregs[0]);

    EXIT;
    return ERR_NONE;
}

///
/// Reserves free pages for the requested address.
/// Note: This doesn't map physical pages to the specified address space;
/// the first access to the requested address generates a page fault.
///
static stat_t
HandlePageAllocate(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    PageFrame       *frame;
    Space           *space;
    Space           *to_space;
    L4_Word_t       dest, count, peerAttr;
    L4_ThreadId_t   peer;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 3)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    dest = L4_Get(msg, 0);
    count = L4_Get(msg, 1);
    peerAttr = L4_Get(msg, 2);

    //
    // 'peer' is the thread that is allowed to share the pages allocated here.
    //
    peer = L4_GlobalId(peerAttr >> 14, Thread::TID_INITIAL_VERSION);

#ifdef SYS_DEBUG
    System.Print("ALLOC for %.8lX @ %.8lX %lu pages peer %.8lX\n",
                 tid.raw, dest, count, peer.raw);
#endif // SYS_DEBUG

    // The allocated page is private.
    if (L4_ThreadNo(peer) == L4_ThreadNo(L4_nilthread)) {
        peer = L4_nilthread;
    }
    // The allocated page is global.
    else if (L4_ThreadNo(peer) == L4_ThreadNo(L4_anythread)) {
        peer = L4_anythread;
    }
    else { // Page sharing

        // Find the space of the peer
        // XXX: Shouldn't _peer_ be the main thread of the space?
        if (FindTask(peer, &to_space) != ERR_NONE) {
            Ipc::ReturnError(msg, ERR_INVALID_THREAD);
        }

#ifdef PEL_DISABLE
        peer = to_space->GetRootThread()->Id;
#else
        if (!to_space->IsShadow()) {
            peer = to_space->GetPager();
        }
#endif
    }

    //
    // Check if the destination is already registered
    //
    for (L4_Word_t i = 0; i < count; i++) {
        if (space->SearchMap(dest + PAGE_SIZE * i, &frame) == ERR_NONE) {
            System.Print("already registered: %.8lX (%.8lX)\n",
                         dest + PAGE_SIZE * i, tid.raw);
            return Ipc::ReturnError(msg, ERR_EXIST);
        }
    }

    if (MainPa.Allocate(count, &frame) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_OUT_OF_MEMORY);
    }

    for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
        PageFrame   *f = frame + i;

        f->SetOwner(space->GetRootThread()->Id);
        f->SetOwnerRights(L4_Label(msg));
        f->SetSharer(peer);
        f->SetSharerRights(peerAttr);
        f->SetDestination(dest);

        space->InsertMap(dest, f);
        dest += PAGE_SIZE;
    }

    EXIT;
    L4_Word_t len = frame->GetPageGroup();
    L4_Put(msg, ERR_NONE, 1, &len, 0, (void *)0);
    return ERR_NONE;
}

static stat_t
HandlePageRelease(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    Space*      space;
    PageFrame*  frame;
    L4_Word_t   address;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    address = L4_Get(msg, 0);
    DOUT("Release %.8lX requested by %.8lX\n", address, tid.raw);

    //
    // Unmap the pages
    //
    if (space->SearchMap(address, &frame) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
        PageFrame   *f = frame + i;
        Pg.Unmap(f, L4_FullyAccessible);
        space->RemoveMap(address + PAGE_SIZE * i);
    }

    // Release the frame in the PF handler if it is shared.
    if (!frame->IsShared()) {
        MainPa.Release(frame);
    }

    return Ipc::ReturnError(msg, ERR_NONE);
}

///
/// Handles page mapping protocol. Maps an anonymous page if source page is
/// not specified.  Maps a read-only page if source page is specified.
///
static stat_t
HandleMapRequest(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    Space*          from_space;
    Space*          to_space;
    PageFrame*      frame;
    L4_Word_t       from_addr;
    L4_Word_t       to_addr;
    L4_Word_t       count;
    L4_Word_t       rwx;
    L4_ThreadId_t   from_sid;
    L4_ThreadId_t   to_sid;
    stat_t          err;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 5)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    to_addr = L4_Get(msg, 0);
    to_addr &= PAGE_MASK;
    to_sid.raw = L4_Get(msg, 1);
    if (FindTask(to_sid, &to_space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    from_addr = L4_Get(msg, 2);
    from_sid.raw = L4_Get(msg, 3);
    count = L4_Get(msg, 4);
    rwx = L4_Label(msg) & PAGE_PERM_MASK;

#ifdef SYS_DEBUG
    System.Print("MAPREQ @ %.8lX (%.8lX)->%.8lX (%.8lX) rwx:%lu, %lu pages\n",
                 from_addr, from_sid.raw, to_addr, to_sid.raw, rwx, count);
#endif // SYS_DEBUG

    if (MAP_REG_LENGTH < count) {
        System.Print(System.ERROR,
                     "Mapping request exceeds the size of registers\n");
        return Ipc::ReturnError(msg, ERR_OUT_OF_RANGE);
    }

    //
    // Map reserved pages
    //
    if (L4_ThreadNo(from_sid) == L4_ThreadNo(to_sid)) {
        DOUT("Search reserved pages\n");
        if (to_space->SearchMap(to_addr, &frame) != ERR_NONE) {
            return Ipc::ReturnError(msg, ERR_NOT_FOUND);
        }

        // Copy on write
        if (frame->IsCOW() && IS_WRITABLE(rwx)) {
            PageFrame   *newf;

            if (MainPa.Allocate(1, &newf) != ERR_NONE) {
                return Ipc::ReturnError(msg, ERR_OUT_OF_MEMORY);
            }

            DOUT("COW %.8lX (%.8lx -> %.8lX)\n",
                 to_addr, Pg.PhysicalAddress(frame),
                 Pg.PhysicalAddress(newf));

            // Remember it's write-accessed.
            frame->AddAccessState(PAGE_STATE_WRITE);

            Pg.CopyPage(newf, frame);
            newf->SetOwner(to_space->GetRootThread()->Id);
            newf->SetOwnerRights(newf->GetOwnerRights() | PAGE_PERM_WRITE);
            newf->SetSharerRights(PAGE_PERM_NONE);
            newf->SetAttribute(0);

            err = Pg.CreateMapItem(to_addr, newf, newf->GetOwnerRights(),
                                   &_mapregs[0]);
            if (err != ERR_NONE) {
                MainPa.Release(newf);
                return Ipc::ReturnError(msg, err);
            }

            // Update the mapping database
            if (to_space->SetMap(to_addr, newf) == FALSE) {
                FATAL("map DB inconsistency");
            }
        }
        else {
            // Arrange the map registers
            err = Pg.CreateMapItem(to_addr, frame, rwx, count, _mapregs);
            if (err != ERR_NONE) {
                return Ipc::ReturnError(msg, err);
            }
        }
    }
    //
    // Map anonymous pages
    //
    else if (L4_ThreadNo(from_sid) == L4_ThreadNo(L4_anythread)
             && from_addr == 0UL) {
        DOUT("Get anonymous pages\n");
        if (to_space->SearchMap(to_addr, &frame) == ERR_NONE) {
            // Copy on write
            if (frame->IsCOW() && IS_WRITABLE(rwx)) {
                PageFrame   *newf;

                if (MainPa.Allocate(1, &newf) != ERR_NONE) {
                    return Ipc::ReturnError(msg, ERR_OUT_OF_MEMORY);
                }

                DOUT("COW %.8lX (%.8lx -> %.8lX)\n",
                     to_addr, Pg.PhysicalAddress(frame),
                     Pg.PhysicalAddress(newf));

                // Remember it's write-accessed.
                frame->AddAccessState(PAGE_STATE_WRITE);

                Pg.CopyPage(newf, frame);
                newf->SetOwner(to_space->GetRootThread()->Id);
                newf->SetOwnerRights(newf->GetOwnerRights() | PAGE_PERM_WRITE);
                newf->SetSharerRights(PAGE_PERM_NONE);
                newf->SetAttribute(0);

                DOUT("owner rights: %lX\n", newf->GetOwnerRights());
                err = Pg.CreateMapItem(to_addr, newf, newf->GetOwnerRights(),
                                       &_mapregs[0]);
                if (err != ERR_NONE) {
                    MainPa.Release(newf);
                    return Ipc::ReturnError(msg, err);
                }
                DOUT("new frame state: %X\n",
                     newf->GetState() | newf->GetAccessState());

                // Update the mapping database
                if (to_space->SetMap(to_addr, newf) == FALSE) {
                    FATAL("map DB inconsistency");
                }
            }
            else {
                // The page is already allocated.
                err = Pg.CreateMapItem(to_addr, frame, rwx, count, _mapregs);
                if (err != ERR_NONE) {
                    MainPa.Release(frame);
                    return Ipc::ReturnError(msg, err);
                }
            }
        }
        // No page is mapped to the destination
        else {
            // Allocate a page
            err = MainPa.Allocate(count, &frame);
            if (err != ERR_NONE) {
                return Ipc::ReturnError(msg, err);
            }

            frame->SetOwner(to_sid);
            frame->SetOwnerRights(rwx);
            frame->SetDestination(to_addr);

            err = Pg.CreateMapItem(to_addr, frame, rwx, count, _mapregs);
            if (err != ERR_NONE) {
                MainPa.Release(frame);
                return Ipc::ReturnError(msg, err);
            }

            for (L4_Word_t i = 0; i < count; i++) {
                Pg.ZeroPage(frame + i);
            }

            // Register the page to the mapping DB
            for (L4_Word_t i = 0; i < count; i++) {
                to_space->InsertMap(to_addr + PAGE_SIZE * i, frame);
            }
        }
    }
    //
    // Direct mapping
    // TODO: I/O page sharing
    else if (L4_IsThreadEqual(from_sid, L4_nilthread)) {
        // Source address outside conventional memory is allowed.
        DOUT("Direct mapping: %.8lX\n", from_addr);
        /*
        if (from_addr < PhysLayout::BIOS_AREA_END) {
            if (MainPa.GetFrameForAddress(from_addr, &frame) != ERR_NONE) {
                return Ipc::ReturnError(msg, ERR_NOT_FOUND);
            }
            err = Pg.CreateMapItem(to_addr, frame, L4_ReadWriteOnly, _mapregs);
            if (err != ERR_NONE) {
                return Ipc::ReturnError(msg, err);
            }
        }
        */

        L4_Fpage_t      fpage1;
        L4_Fpage_t      fpage2;
        L4_Word_t       reg[2];
        L4_Msg_t        m;
        L4_Word_t       indirect_page;
       
        err = IoPa.Allocate(1, &indirect_page);
        if (err != ERR_NONE) {
            return Ipc::ReturnError(msg, err);
        }

        fpage1 = L4_FpageLog2(from_addr, PAGE_BITS);
        fpage2 = L4_FpageLog2(indirect_page, PAGE_BITS);
        L4_Set_Rights(&fpage1, L4_FullyAccessible);
        L4_Set_Rights(&fpage2, L4_FullyAccessible);

        // Request Sigma0 map
        reg[0] = fpage1.raw;
        reg[1] = 0;
        L4_Put(&m, MSG_SIGMA0, 2, reg, 0, 0);
        L4_Accept(L4_MapGrantItems(fpage2));
        err = Ipc::Call(L4_Pager(), &m, &m);
        if (err != ERR_NONE) {
            return Ipc::ReturnError(msg, err);
        }
        L4_Accept(L4_MapGrantItems(L4_Nilpage));
        //L4_Get(&m, 0, &tmp);

        _mapregs[0] = L4_MapItem(fpage2, to_addr);
    }
    //
    // Map shared pages owned by from_sid
    // NOTE: from_sid must be PEL
    //
    // TODO: Make sure the address held by the page frame object won't
    //       be changed.
    else {
        DOUT("page sharing\n");
        if (FindTask(from_sid, &from_space) != ERR_NONE) {
            return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
        }

#ifndef PEL_DISABLE
        if (!L4_IsThreadEqual(from_space->GetPager(), L4_Myself())) {
            // Overwrite the source address space
            if (FindTask(from_space->GetPager(), &from_space) != ERR_NONE) {
                return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
            }
        }
#endif
        for (L4_Word_t i = 0; i < count; i++) {
            if (to_space->SearchMap(to_addr + PAGE_SIZE * i, &frame) ==
                    ERR_NONE) {
                Pg.Unmap(frame, L4_FullyAccessible);
                to_space->RemoveMap(to_addr + PAGE_SIZE * i);
                MainPa.Release(frame);
            }
        }

        for (L4_Word_t i = 0; i < count; i++) {
            if (from_space->SearchMap(from_addr + PAGE_SIZE * i, &frame) !=
                    ERR_NONE) {
                System.Print(System.ERROR,
                             "address is not registered in the source\n");
                return Ipc::ReturnError(msg, ERR_NOT_FOUND);
            }

            //
            // The frame to be mapped must be owned by from_sid
            //
            if (L4_ThreadNo(frame[i].GetOwner()) !=
                    L4_ThreadNo(from_space->GetRootThread()->Id)) {
                System.Print(System.ERROR, "Invalid owner\n");
                DOUT("Invalid owner\n");
                return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
            }

            //
            // Check authentication
            //
            L4_ThreadId_t peer = frame[i].GetSharer();
            if (L4_ThreadNo(peer) != L4_ThreadNo(L4_anythread) && 
                L4_ThreadNo(peer) !=
                    L4_ThreadNo(to_space->GetRootThread()->Id)) {
                System.Print(System.ERROR,
                             "invalid authentication: %.8lX %.8lX\n",
                             peer.raw, to_space->GetRootThread()->Id.raw);
                DOUT("invalid authentication: %.8lX %.8lX\n",
                     peer.raw, to_space->GetRootThread()->Id.raw);
                return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
            }

            //
            // Check permission
            //
            if (((rwx ^ frame[i].GetSharerRights()) &
                 ~frame[i].GetSharerRights()) != 0) {
                System.Print(System.ERROR, "invalid rights: %.8lX %.8lX\n",
                             rwx, frame[i].GetSharerRights());
                DOUT("invalid rights: %.8lX %.8lX\n",
                     rwx, frame[i].GetSharerRights());
                return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
            }
        }

        //
        // Map
        //
        err = Pg.CreateMapItem(to_addr, frame, rwx, count, _mapregs);
        if (err != ERR_NONE) {
            MainPa.Release(frame);
            return Ipc::ReturnError(msg, err);
        }

        for (L4_Word_t i = 0; i < count; i++) {
            to_space->InsertMap(to_addr, &frame[i]);
            to_addr += PAGE_SIZE;
        }
    }

    L4_Put(msg, 0, 0, 0, 2 * count, _mapregs);

    EXIT;
    return ERR_NONE;
}

//TODO: Partial unmapping
static stat_t
HandleUnmapRequest(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    PageFrame*      frame;
    Space*          space;
    L4_Word_t       address, rwx, attr;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    address = L4_Get(msg, 0);

#ifdef SYS_DEBUG
    System.Print(System.INFO, "UNMAPREQ @ %.8lX space %.8lX from %.8lX\n",
                 address, space->GetRootThread()->Id.raw, tid.raw);
#endif // SYS_DEBUG

    if (space->SearchMap(address, &frame) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    rwx = L4_Label(msg) & PAGE_PERM_MASK;
    attr = frame->GetSharerRights();
    if (((rwx ^ attr) & ~attr) == 0) {
        // Not owner, return.
        return Ipc::ReturnError(msg, ERR_NONE);
    }

    // Owner of the region
    if (L4_IsThreadEqual(frame->GetOwner(), tid) != ERR_NONE) {
        DOUT("%.8lX is owner\n", tid.raw);
        for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
            Pg.Unmap(&frame[i], rwx);
            address += PAGE_SIZE;

            // NOTE: Don't remove the address from the mapping DB here,
            // even though it is full unmapping (discarding the full access
            // permission).  To remove the address from the mapping DB,
            // the owner must send a release message, then the corresponding
            // addresses are removed from the DB.
        }
    }
    // NOTE: Unmapping discards the mapping of the page to all spaces (you
    // can't select and unmap any destination).  So we expect the unmapped
    // page is remapped by a page fault at each space except the space
    // unmapped here.
    else {
        DOUT("%.8lX is not owner (%.8lX)\n", tid.raw, frame->GetOwner().raw);
        for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
            Pg.Unmap(&frame[i], rwx);
            //FIXME:
            //if (rwx == attr) {
                space->RemoveMap(address);
            //}
            address += PAGE_SIZE;
        }
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

// Maps the requested physical address to the destination address
static stat_t
HandleIoMap(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    /*
    Space*      space;
    L4_Word_t   dest;
    L4_Word_t   phys;
    L4_Word_t   count;
    stat_t      err;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 3)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    dest = L4_Get(msg, 0);
    phys = L4_Get(msg, 1);
    count = L4_Get(msg, 2);
    rwx = L4_Label(msg) & PAGE_PERM_MASK;

    if (MAP_REG_LENGTH < count) {
        return Ipc::ReturnError(msg, ERR_OUT_OF_RANGE);
    }

    if ((rwx & L4_eXecutable) != 0) {
        return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    err = IoPg.CreateMapItem(dest, phys, rwx, count, _mapregs);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    L4_Put(msg, 0, 0, 0, count * 2, _mapregs);

    EXIT;
    return ERR_NONE;
    */
    return ERR_UNKNOWN;
}

static stat_t
HandleIoUnmap(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    /*
    PageFrame   *frame;
    Space       *space;
    L4_Word_t   address;
    L4_Word_t   attr;
    L4_Word_t   count;
    L4_Word_t   rwx;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    address = L4_MsgWord(msg, 0);
    count = L4_MsgWord(msg, 1);

    if (space->SearchMap(address, &frame) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    rwx = L4_Label(msg) & PAGE_PERM_MASK;
    attr = frame->GetSharerRights() & PAGE_PERM_MASK;
    if (((rwx ^ attr) & ~attr) == 0) {
        return Ipc::ReturnError(msg, ERR_NONE);
    }

    // Owner of the region
    if (L4_IsThreadEqual(frame->GetOwner(), tid) != ERR_NONE) {
        for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
            IoPg.Unmap(&frame[i], rwx);
            address += PAGE_SIZE;

            // NOTE: Don't remove the address from the mapping DB here,
            // even though it is full unmapping (discarding the full access
            // permission).  To remove the address from the mapping DB,
            // the owner must send a release message, then the corresponding
            // addresses are removed from the DB.
        }
    }
    // NOTE: Unmapping discards the mapping of the page to all spaces (you
    // can't do select a destination and unmap only it).  So we expect the
    // unmapped page is remapped by a page fault at each space except the
    // specified one.
    else {
        for (L4_Word_t i = 0; i < frame->GetPageGroup(); i++) {
            IoPg.Unmap(&frame[i], rwx);
            if (rwx == attr) {
                space->RemoveMap(address);
            }
            address += PAGE_SIZE;
        }
    }

    return Ipc::ReturnError(msg, ERR_NONE);
    */
    return ERR_UNKNOWN;
}

static stat_t
HandlePhys(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t address;
    PageFrame *frame;
    Space *space;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    address = L4_MsgWord(msg, 0);

    if (space->SearchMap(address, &frame) == ERR_NONE) {
        address = Pg.PhysicalAddress(frame);
    }
    else {
        address = ~0UL;
    }

    L4_Put(msg, 0, 1, &address, 0, 0);

    EXIT;
    return ERR_NONE;
}

/**
 * Map a BIOS reserved area page.
 */
static stat_t
HandleMapBiosPage(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t address, biosaddress;
    Space *space;
    stat_t err;
    PageFrame *page;
    
    ENTER;
	
    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }
    
    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }
    
    biosaddress = L4_MsgWord(msg, 0);
    
    // The only authorized pages are the first page (which contains the
    // interrupts handlers table) and the area starting from 0xa0000 
    // (BIOS mapped memory) and ending at 0x100000 (end of first megabyte)
    if ((biosaddress > 0x1000 && biosaddress < 0x9f000) ||
        biosaddress >= 0x100000) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }
    
    address = L4_MsgWord(msg, 1);
    
    if (MainPa.GetFrameForAddress(biosaddress, &page) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }
    
    err = Pg.CreateMapItem(address, page, L4_ReadWriteOnly, _mapregs);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }
    
    L4_Put(msg, ERR_NONE, 0, 0, 2, _mapregs);
    EXIT;
    return ERR_NONE;
}

