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
/// @brief  IPC handler of P3
/// @file   Services/P3/IpcHandler.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: IpcHandler.cc 427 2008-11-01 01:42:45Z hro $

#include <l4/kdebug.h>
#include <Debug.h>
#include <FileStream.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <System.h>
#include <Types.h>
#include <l4/types.h>
#include <FaultInjection.h>
#include <Random.h>

#include "IpcHandler.h"
#include "Pel.h"
#include "Segment.h"
#include "Task.h"
#include "Loader.h"


stat_t
IpcHandler::HandleIpc(Task *t)
{
    L4_ThreadId_t   tid;
    L4_MsgTag_t     tag;
    L4_Msg_t        msg;
    stat_t          err = ERR_UNKNOWN;

    _task = t;

begin:
    //TODO: check the source TID
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
                    err = Ipc::ErrorCode();
                    goto exit;
                case ERR_IPC_OVERFLOW:
                default:
                    BREAK("ipc error");
                    goto begin;
            }
        }

        L4_Store(tag, &msg);
        L4_Word_t label = L4_MsgLabel(&msg);
        switch (label & MSG_MASK) {
            case MSG_PEL_START_TH:
                err = HandleStartThread(&msg);
                break;
            case MSG_PEL_CANCEL:
                err = HandleCancel(&msg);
                break;
            case MSG_PEL_RESTART:
                err = HandlePelRestart(tid, &msg);
                goto begin;
            case MSG_PEL_READY:
                _task->user_state = Task::USER_READY;
                goto begin;
            case MSG_PEL_INJECT:
                err = HandleInject(tid, &msg);
                goto begin;
                break;

            case MSG_PAGER_PF:
                err = HandlePageFault(tid, &msg);
                if (err == ERR_FATAL) {
                    goto exit;
                }
                break;
            case MSG_PAGER_ALLOCATE:
                err = HandleAllocate(&msg);
                break;
            case MSG_PAGER_RELEASE:
                err = HandleRelease(&msg);
                if (err == ERR_NOT_FOUND) {
                    err = ERR_NONE;
                }
                break;
            case MSG_PAGER_EXPAND:
                err = HandleExpandHeap(&msg);
                break;
            case MSG_PAGER_MAP:
                err = HandleMap(&msg);
                break;
            case MSG_PAGER_UNMAP:
            case MSG_PAGER_PHYS:
                err = Ipc::Call(L4_Pager(), &msg, &msg);
                if (err == ERR_NOT_FOUND) {
                    err = ERR_NONE;
                }
                break;
            case MSG_PAGER_MAPBIOSPAGE:
                err = HandleMapBios(tid, &msg);
                break;
            case MSG_ROOT_NEW_TH:
                err = HandleCreateThread(tid, &msg);
                break;
            case MSG_ROOT_DEL_TH:
                err = HandleDeleteThread(tid, &msg);
                break;
            case MSG_ROOT_PRIO:
            case MSG_ROOT_SET_INT:
            case MSG_ROOT_UNSET_INT:
            case MSG_ROOT_EXEC:
            case MSG_ROOT_KILL:
            case MSG_ROOT_DEL_TSK:
            case MSG_ROOT_RESTART:
            case MSG_ROOT_INJECT:
                err = Ipc::Call(Pel::RootTask(), &msg, &msg);
                break;
            case MSG_ROOT_NS:
                err = HandleGetNs(&msg);
                break;
            case MSG_ROOT_EXIT:
                err = HandleExit(&msg);
                goto exit;
            default:
                System.Print(System.WARN, "PEL(%.8lX): "
                             "Unknown message %lX from %.8lX err %lu\n",
                             L4_Myself().raw, L4_Label(tag), tid.raw,
                             L4_ErrorCode());
                System.Print(System.WARN, "%.8lX  %.8lX  %.8lX\n",
                             msg.raw[0], msg.raw[1], msg.raw[2]);
                L4_Clear(&msg);
                L4_KDB_Enter("debug");
                break;
        }

        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }
exit:
    return err;
}

stat_t
IpcHandler::HandleStartThread(L4_Msg_t *msg)
{
    L4_ThreadId_t   tid;
    L4_Word_t       reg[2];
    L4_Msg_t        start_msg;
    stat_t          err = ERR_UNKNOWN;

    ENTER;

    tid.raw = L4_MsgWord(msg, 0);
    reg[0] = L4_MsgWord(msg, 1);
    reg[1] = L4_MsgWord(msg, 2);
    //System.Print(DEBUG, "Thread start TID: %.8lX IP: %.8lX SP: %.8lX\n",
    //           tid.raw, reg[0], reg[1]);
    //err = StartThread(tid, ip, sp);

    L4_Put(&start_msg, 0, 2, reg, 0, 0);
    err = Ipc::Send(tid, &start_msg);

    EXIT;
    return Ipc::ReturnError(msg, err);
}

stat_t
IpcHandler::HandleCancel(L4_Msg_t* msg)
{
    L4_Word_t       control;
    L4_Word_t       dummy;
    L4_ThreadId_t   tid;
    L4_ThreadId_t   dummy_tid;
    ENTER;

    tid.raw = L4_Get(msg, 0);

    // Cancel sending and receiving IPC
    control = (1 << 2) | (1 << 1);

    L4_ExchangeRegisters(tid, control, 0, 0, 0, 0, L4_nilthread,
                         &dummy, &dummy, &dummy, &dummy, &dummy, &dummy_tid);
    L4_ThreadSwitch(tid);
    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

stat_t
IpcHandler::HandlePelRestart(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    L4_Word_t time = L4_Get(msg, 0);

    L4_Sleep(L4_TimePeriod(time * 1000));

    //_exit_code = EXIT_RESTART;
    _exit_code = 2;

    return ERR_NONE;
}

stat_t
IpcHandler::RecoverCheckpoint(L4_ThreadId_t tid, L4_Word_t generation)
{
    /*
    L4_Msg_t msg;

    L4_Put(&msg, MSG_ROOT_RESTORE, 1, &generation, 0, 0);
    stat_t err = Ipc::Call(Pel::RootTask(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    L4_Word_t ip = L4_Get(&msg, 0);
    L4_Word_t sp = L4_Get(&msg, 1);

    System.Print(System.INFO,
                 "Pel: Restore IP: %.8lX SP: %.8lX\n", ip, sp);

    // Restore IP and SP
    L4_Word_t       dummy;
    L4_ThreadId_t   dummy_tid;
    L4_Word_t       control = (1 << 4) | (1 << 3);
    L4_ExchangeRegisters(tid, control, sp, ip, 0, 0, L4_nilthread,
                         &dummy, &dummy, &dummy, &dummy, &dummy,
                         &dummy_tid);

    // Initialize the stack segment
    _task->stack.Initialize(USER_STACK_END);
    */

    return ERR_NONE;
}

stat_t
IpcHandler::RecoverLazy()
{
    /*
    L4_Word_t       control = 1 << 4;
    L4_Word_t       dummy;
    L4_ThreadId_t   dummy_tid;
    L4_Word_t       restart;
    UByte*          ip = reinterpret_cast<UByte*>(fip);

    // mov instruction
    if (*ip == 0x07) {
        restart = fip + 7;
        ip++;
        if (*ip == 0x04) {
            restart += 5;
        }
        else if (*ip == 0x05) {
            restart += 3;
        }
        else if (*ip == 0x44) {
            restart += 1;
        }
    }

    System.Print(System.INFO,
                 "Ignore the failure: Restore IP: %.8lX\n", restart);
    // Skip the mov instruction
    L4_ExchangeRegisters(tid, control, 0, restart, 0, 0, L4_nilthread,
                         &dummy, &dummy, &dummy, &dummy, &dummy,
                         &dummy_tid);
                         */
    return ERR_NONE;
}

stat_t
IpcHandler::RecoverURB()
{
    /*
    TaskLoader  loader;
    loader.Load(_task);

    L4_Word_t       args[2];
    L4_Word_t       dummy;
    L4_ThreadId_t   dummy_tid;

    args[0] = _task->heap.StartAddress();
    args[1] = _task->pm.StartAddress();
    L4_Word_t sp = _task->stack.SetupArgs(args, 2);
    L4_Word_t ip = _task->entry;
    L4_Word_t control = (1 << 4) | (1 << 3);
    L4_ExchangeRegisters(tid, control, sp, ip, 0, 0, L4_nilthread,
                         &dummy, &dummy, &dummy, &dummy, &dummy,
                         &dummy_tid);
                         */

    return ERR_NONE;
}

stat_t
IpcHandler::Recover()
{
    /*
    RecoverLazy();
    RecoverURB();
    */
    return ERR_FATAL;
}

stat_t
IpcHandler::HandlePageFault(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t       faddr;
    L4_Word_t       fip;
    L4_Word_t       rwx;
    Segment         *segment;
    stat_t        stat;
    ENTER;

    faddr = L4_Get(msg, 0) & PAGE_MASK;
    fip = L4_Get(msg, 1);
    rwx = L4_Label(msg) & 0xF;

    DOUT("PF @ %.8lX, ip %.8lX, rwx %lu\n", faddr, fip, rwx);

    segment = _task->GetSegment(faddr);
    if (segment == 0) {
        System.Print("address out of range\n");
        System.Print("PF @ %.8lX, ip %.8lX, rwx %lu\n", faddr, fip, rwx);
        //return ERR_OUT_OF_RANGE;
        return ERR_FATAL;
    }

    stat = segment->HandlePf(tid, faddr, fip, &rwx);

    if (stat == ERR_NONE) {
        L4_Fpage_t      fpage;
        L4_MapItem_t    map;

        //
        // Map the page from PEL to the user task
        //
        fpage = L4_FpageLog2(faddr, PAGE_BITS);
        L4_Set_Rights(&fpage, rwx);
        map = L4_MapItem(fpage, faddr);
        L4_Put(msg, 0, 0, (L4_Word_t *)0, 2, &map);
    }
    else {
        System.Print("Invalid address\n");
        System.Print("PF @ %.8lX, ip %.8lX, rwx %lu\n", faddr, fip, rwx);
        stat = Recover();
    }

    EXIT;
    return stat;
}

stat_t
IpcHandler::HandleExpandHeap(L4_Msg_t *msg)
{
    L4_Word_t   count;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    count = L4_Get(msg, 0);

    _task->heap.Grow(count);
    if (_task->stack.StartAddress() <= _task->heap.EndAddress()) {
        //TODO: rewind the growing
        return Ipc::ReturnError(msg, ERR_OUT_OF_MEMORY);
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

stat_t
IpcHandler::HandleAllocate(L4_Msg_t *msg)
{
    L4_Word_t       address;
    stat_t        err;
    ENTER;

    address = L4_Get(msg, 0);

    if (_task->heap.Hit(address)) {
        err = Ipc::Call(L4_Pager(), msg, msg);
    }
    else if (_task->shm.Hit(address)) {
        err = Ipc::Call(L4_Pager(), msg, msg);
    }
    else {
        err = Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    EXIT;
    return Ipc::ReturnError(msg, err);
}

///
/// Releases the pages
///
stat_t
IpcHandler::HandleRelease(L4_Msg_t *msg)
{
    stat_t err = Ipc::Call(L4_Pager(), msg, msg);
    return Ipc::ReturnError(msg, err);
}

///
/// Allocates the count of pages, and map the same number of the continuous
/// pages in the root task to them.
///
stat_t
IpcHandler::HandleMap(L4_Msg_t *msg)
{
    L4_Word_t       dest, count, rwx;
    L4_Word_t       src;
    L4_Word_t       reg[5];
    L4_ThreadId_t   did;
    L4_ThreadId_t   sid;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 5)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    L4_MsgGet(msg, reg, (void *)0);
    dest = reg[0];
    did.raw = reg[1];
    src = reg[2];
    sid.raw = reg[3];
    count = reg[4];
    rwx = L4_MsgLabel(msg) & 0x7;

    DOUT("dst:%.8lX@%.8X, src:%.8lX@%.8lX, %lu pages, %lu\n",
         dest, did.raw, src, sid.raw, count, rwx);

    //XXX
    if (dest < 0x80000000UL) {
        if (!_task->heap.Hit(dest)) {
            return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
        }
    }

    // Request for an anonymous physical page
    //if (reg[2] == PAGE_MASK) {
    if (sid == L4_Myself()) {
        sid = L4_Pager();
    }

    //
    // Map the source to the destination
    //
    Pager.Map(dest, rwx, src, sid);

    //
    // Map the pages to the task's AS
    //
    for (L4_Word_t i = 0; i < count; i++) {
        L4_Fpage_t  fpage;
        L4_Word_t   offset = PAGE_SIZE * i;

        fpage = L4_FpageLog2(dest + offset, PAGE_BITS);
        L4_Set_Rights(&fpage, L4_ReadWriteOnly);
        _mapregs[i] = L4_MapItem(fpage, dest + offset);
    }

    L4_Put(msg, 0, 0, (L4_Word_t *)0, count * 2, _mapregs);

    EXIT;
    return ERR_NONE;
}

stat_t
IpcHandler::HandleIoMap(L4_Msg_t *msg)
{
    L4_Word_t       phys;
    L4_Word_t       virt;
    L4_Word_t       dest;
    L4_Word_t       count;
    L4_MapItem_t    item;
    L4_Word_t       reg[2];
    stat_t          err;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 3)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    dest = L4_Get(msg, 0);
    phys = L4_Get(msg, 1);
    count = L4_Get(msg, 2);

    if (!_task->heap.Hit(dest)) {
        return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));

    reg[0] = phys;
    reg[1] = dest;
    L4_Put(msg, MSG_PAGER_IOMAP, 2, reg, 0, 0);
    err = Ipc::Call(L4_Pager(), msg, msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    L4_MsgGetMapItem(msg, 0, &item);
    virt = L4_MapItemSndBase(item);

    for (L4_Word_t i = 0; i < count; i++) {
        L4_Fpage_t  fpage;
        L4_Word_t   offset = PAGE_SIZE * i;

        fpage = L4_FpageLog2(virt + offset, PAGE_BITS);
        L4_Set_Rights(&fpage, L4_ReadWriteOnly);
        _mapregs[i] = L4_MapItem(fpage, virt + offset);
    }

    L4_Put(msg, 0, 0, 0, count * 2, _mapregs);

    EXIT;
    return ERR_NONE;
}

stat_t
IpcHandler::HandleCreateThread(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t reg;
    stat_t err;
    ENTER;

    reg = tid.raw;
    L4_Put(msg, MSG_ROOT_NEW_TH, 1, &reg, 0, 0);
    err = Ipc::Call(Pel::RootTask(), msg, msg);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    reg = L4_Get(msg, 0);
    L4_Put(msg, ERR_NONE, 1, &reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

stat_t
IpcHandler::HandleDeleteThread(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    L4_Word_t   reg[2];
    stat_t      err;
    ENTER;

    reg[0] = L4_Get(msg, 0);
    reg[1] = tid.raw;
    L4_Put(msg, MSG_ROOT_DEL_TH, 2, reg, 0, 0);
    err = Ipc::Call(Pel::RootTask(), msg, msg);
    return Ipc::ReturnError(msg, err);
}

stat_t
IpcHandler::HandleMapBios(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t   address;
    L4_Fpage_t  fpage;
    stat_t      err;

    address = L4_Get(msg, 1);
    
    // Get the pages from the root pager
    fpage = L4_FpageLog2(address, PAGE_BITS);
    L4_Set_Rights(&fpage, L4_ReadWriteOnly);

    L4_Accept(L4_MapGrantItems(fpage));
    err = Ipc::Call(L4_Pager(), msg, msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));

    if (err != ERR_NONE) {
        return err;
    }
    
    // Share the page with the user process
    L4_MapItem_t mapitem = L4_MapItem(fpage, address & PAGE_MASK);
    L4_Put(msg, ERR_NONE, 0, 0, 2, &mapitem);
    return ERR_NONE;
}

stat_t
IpcHandler::HandleMapLFB(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t spaceaddress = L4_Get(msg, 1);
    
    L4_Fpage_t fpage = L4_FpageLog2(spaceaddress, PAGE_BITS);
    L4_Set_Rights(&fpage, L4_ReadWriteOnly);
    
    // Get the pages from the root pager
    L4_Accept(L4_MapGrantItems(fpage));
    stat_t err = Ipc::Call(L4_Pager(), msg, msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    if (err != ERR_NONE) return err;
    
    // Share the page with the user process
    L4_MapItem_t mapitem = L4_MapItem(fpage, spaceaddress & PAGE_MASK);
    L4_Put(msg, ERR_NONE, 0, 0, 2, &mapitem);
    return ERR_NONE;
}

stat_t
IpcHandler::HandleGetNs(L4_Msg_t *msg)
{
    ENTER;

    L4_Word_t reg;
    reg = Pel::RootTask().raw;
    L4_Put(msg, ERR_NONE, 1, &reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

stat_t
IpcHandler::HandleExit(L4_Msg_t *msg)
{
    ENTER;
    _exit_code = L4_Get(msg, 0);
    DOUT("User exit code: %u\n", _exit_code);
    EXIT;
    return ERR_NONE;
}

stat_t
IpcHandler::HandleInject(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    L4_Word_t   where;
    L4_Word_t   old;
    L4_Word_t*  cur;
    addr_t      start_addr;
    addr_t      end_addr;
    Segment*    seg;

    where = L4_Get(msg, 0);

    if (where == FI_TEXT_AREA) {
        seg = &_task->text;
    }
    else if (where == FI_DATA_AREA) {
        seg = &_task->data;
    }

    start_addr = seg->StartAddress();
    end_addr = seg->EndAddress();

    // Flip the data
    cur = (L4_Word_t*)(rand() % (end_addr - start_addr) + start_addr);
    old = *cur;
    *cur &= *cur >> 1;

    System.Print("FI->%.8lX@%p %.8lX->%.8lX\n",
                 _task->main_tid.raw, cur, old, *cur);

    return ERR_NONE;
}

