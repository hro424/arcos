/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @file   Services/Devices/Sata/Runtime.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Runtime.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/ipc.h>
#include <arc/memory.h>
#include <arc/mutex.h>
#include <arc/protocol.h>
#include <arc/system.h>
#include <arc/status.h>
#include <arc/string.h>
#include "Sata.h"
#include "Runtime.h"
#include "Ata.h"
#include "AtaCommandBlock.h"

#include <l4/ipc.h>
#include <l4/types.h>
#include <l4/message.h>
#include <l4/schedule.h>

static const UInt       INTR_STACK_PAGES = 2;
static Sata             *_device;
static L4_ThreadId_t    _intrThread;
static L4_Word_t        _intrStack;

//-----------------------------------------------------------------------------
//      Server routines
//-----------------------------------------------------------------------------

struct Session {
    Session         *next;
    Session         *prev;
    L4_ThreadId_t   tid;
    addr_t          address;
    UInt            count;     // page count
    UInt            device;
};

static Session _head;
static Mutex _listMutex;

static void Add(Session *s)
{
    _listMutex.Lock();
    s->next = _head.next;
    s->prev = &_head;
    _head.next->prev = s;
    _head.next = s;
    _listMutex.Unlock();
}

static void
Del(Session *s)
{
    _listMutex.Lock();
    s->prev->next = s->next;
    s->next->prev = s->prev;
    _listMutex.Unlock();
}

static Session *
Find(L4_ThreadId_t t)
{
    Session *cur;
    Session *ret = 0;

    _listMutex.Lock();
    for (cur = _head.next; cur != &_head; cur = cur->next) {
        if (L4_IsThreadEqual(cur->tid, t)) {
            ret = cur;
            break;
        }
    }
    _listMutex.Unlock();

    return ret;
}


static status_t Open(L4_ThreadId_t tid, L4_Msg_t *msg);
static status_t Close(L4_ThreadId_t tid, L4_Msg_t *msg);
static status_t Read(L4_ThreadId_t tid, L4_Msg_t *msg);
static status_t Write(L4_ThreadId_t tid, L4_Msg_t *msg);

static void
HandleIpc()
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    L4_ThreadId_t   peer;
    status_t        stat;

    _device->Init();
    //_device->Test();

    tag = L4_Wait(&peer);
    for (;;) {
        L4_Store(tag, &msg);

        switch (L4_Label(&msg) & MSG_MASK) {
            case MSG_SESSION_CONNECT:
                break;
            case MSG_SESSION_DISCONNECT:
                break;
            case MSG_SESSION_BEGIN:
                stat = Open(peer, &msg);
                break;
            case MSG_SESSION_END:
                stat = Close(peer, &msg);
                break;
            case MSG_SESSION_GET:
                stat = Read(peer, &msg);
                break;
            case MSG_SESSION_PUT:
                stat = Write(peer, &msg);
                break;
            default:
                ConsoleOut(WARN, "Sata: unknown message %lu from %.8lX\n",
                           L4_Label(&msg), peer.raw);
                BREAK();
                break;
        }

        L4_Load(&msg);

        tag = L4_ReplyWait(peer, &peer);
    }
}

// Receives a pointer to the shared space and maps to it
static status_t
Open(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t       reg[2];
    L4_MapItem_t    item;
    Session         *session;
    status_t        err;

    ENTER;

    reg[0] = L4_Get(msg, 0);    // device ID

    _device->Open(reg[0]);

    session = (Session *)malloc(sizeof(Session));
    session->device = reg[0];

    reg[0] = L4_Get(msg, 1);    // address, order
    reg[1] = L4_Get(msg, 2);    // space ID

    L4_Put(msg, MSG_PEL_MAP | L4_ReadWriteOnly, 2, reg, 0, (void *)0);

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));
    err = IpcCall(L4_Pager(), msg, msg);
    if (err != ERR_NONE) {
        mfree(session);
        return err;
    }

    L4_Get(msg, 0, &item);

    session->tid = tid;
    session->address = L4_Address(L4_SndFpage(item));
    session->count = L4_TypedWords(L4_MsgTag(msg)) / 2;

    Add(session);

    DOUT("Session %.8lX %.8lX\n", session->address, session->count);

    L4_Clear(msg);
    EXIT;

    return ERR_NONE;
}

static status_t
Close(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    Session *session;

    ENTER;
    // Unmap the shared pages

    session = Find(tid);
    Del(session);

    _device->Close(session->device);
    mfree(session);

    EXIT;
    return ERR_NONE;
}

static status_t
Read(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    AtaCommandBlock cb;
    UInt sectors;
    UInt loc;
    L4_Word_t reg;
    Session *session;
    ENTER;

    session = Find(tid);

    // Read data from the device and write it into the shared space
    loc = L4_Get(msg, 0);
    sectors = L4_Get(msg, 1);
    //buffer = L4_Get(msg, 2);
    //length = L4_Get(msg, 3);


    cb.Initialize();
    cb.count = sectors;
    cb.SetLba(loc);
    cb.command = ATA_CMD_READ_DMA_R;
    DOUT("device: %lu, location: %lu, sectors: %u\n",
              session->device, cb.Lba(), cb.count);
    DOUT("session addr: %.8lX page count: %lu\n",
              session->address, session->count);
    _device->Read(session->device, &cb, (void *)session->address,
                  session->count * PAGE_SIZE);

    /*
    printf("-- Dump Sata Read --\n");
    char *ptr = (char *)session->address;
    for (int i = 0; i < cb.count * 256; i += 32) {
        for (int j = i; j < i + 32; j++) {
            printf("%.2X ", ptr[j] & 0xFF);
        }
        printf("\n");
    }
    printf("-- Dump Sata Read End --\n");
    */

    reg = 0;
    L4_Put(msg, 0, 1, &reg, 0, (void *)0);

    EXIT;
    return ERR_NONE;
}

static status_t
Write(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    AtaCommandBlock cb;
    UInt            sectors;
    UInt            loc;
    size_t          length;
    L4_Word_t       buffer;
    Session         *session;

    ENTER;

    session = Find(tid);

    // Write the data in the shared space to the device
    loc = (UInt)L4_Get(msg, 0);
    sectors = (UInt)L4_Get(msg, 1);
    buffer = L4_Get(msg, 2);
    length = (size_t)L4_Get(msg, 3);
    
    cb.Initialize();
    cb.count = sectors;
    cb.SetLba(loc);
    cb.command = ATA_CMD_WRITE_DMA_R;
    _device->Write(session->device, &cb, (void *)buffer, length);

    EXIT;
    return ERR_NONE;
}

void
StartDevice(Sata *obj)
{
    _device = obj;

    _head.tid = L4_nilthread;
    _head.next = &_head;
    _head.prev = &_head;


    HandleIpc();
}



//-----------------------------------------------------------------------------
//      Interrupt Handling Routines
//-----------------------------------------------------------------------------

static void
HandleInterrupts()
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    L4_ThreadId_t   tid;

    ConsoleOut(INFO, "Starting interrupt thread (%.8lX)...\n",
               L4_Myself().raw);

    tag = L4_Wait(&tid);
    for (;;) {
        L4_Store(tag, &msg);

        DOUT("Interrupt from %.8lX\n", tid.raw);
        if (_device->HandleInterrupts(tid, &msg) != ERR_NONE) {
            break;
        }

        L4_Clear(&msg);
        L4_Load(&msg);
        L4_ReplyWait(tid, &tid);
    }
}


static status_t
CreateIntThread()
{
    L4_Msg_t    msg;
    L4_Word_t   reg[2];
    L4_Word_t   i;
    status_t    err;

    //reg[0] = L4_Pager().raw;
    //L4_Put(&msg, MSG_ROOT_NEW_TH, 1, reg, 0, (void *)0);
    L4_Put(&msg, MSG_ROOT_NEW_TH, 0, (L4_Word_t *)0, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        ConsoleOut(ERROR, "thread creation failed\n");
        return err;
    }

    _intrThread.raw = L4_Get(&msg, 0);

    for (i = 0; (1UL << i) < INTR_STACK_PAGES; i++) ;

    reg[0] = i;
    reg[1] = L4_ThreadNo(L4_Pager()) << 14 | L4_ReadWriteOnly;
    L4_Put(&msg, MSG_PEL_ALLOCATE, 2, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        /*
        L4_Put(&msg, MSG_ROOT_DEL_TH, 1, reg, 0, (void *)0);
        if (IpcCall(_cs, &msg, &msg) != ERR_NONE) {
            printf(ERROR, "thread deletion failed\n");
        }
        */
        ConsoleOut(ERROR, "stack allocation failed\n");
        return err;
    }

    _intrStack = L4_Get(&msg, 0);
    _intrStack += PAGE_SIZE * INTR_STACK_PAGES - 1; 

    /*
    reg[0] = L4_Myself().raw;
    reg[1] = 0xFF;  // highest priority
    L4_Put(&msg, MSG_ROOT_PRIO, 2, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        printf(ERROR, "priority is not set\n");
        return err;
    }
    */

    DOUT("interrupt thread stack @ %.8lX\n", _intrStack);

    return ERR_NONE;
}

static status_t
RunIntThread()
{
    L4_Msg_t    msg;
    L4_Word_t   reg[3];
    status_t    err;

    reg[0] = _intrThread.raw;
    reg[1] = (L4_Word_t)HandleInterrupts;
    reg[2] = _intrStack;
    L4_Put(&msg, MSG_PEL_START_TH, 3, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    L4_ThreadSwitch(_intrThread);

    return ERR_NONE;
}

/*
addr_t
GetPhys(addr_t virt)
{
    L4_Msg_t    msg;
    L4_Word_t   reg[2];
    L4_Word_t   offset;
    status_t    err;

    reg[0] = (L4_Word_t)virt & PAGE_MASK;
    offset = (L4_Word_t)virt & ~PAGE_MASK;
//    reg[1] = sid.raw;
    L4_Put(&msg, MSG_PEL_PHYS, 2, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return 0UL;
    }

    reg[0] = L4_Get(&msg, 0);

    return reg[0] + offset;
}
*/

status_t
SetInterrupt(UInt irq)
{
    L4_Msg_t        msg;
    L4_ThreadId_t   tid;
    status_t        err;

    ENTER;

    // Start the interrupt handling thread
    CreateIntThread();

    // Register IHT
    tid = L4_GlobalId(L4_ThreadNo(_intrThread), irq);
    L4_Put(&msg, MSG_ROOT_SET_INT, 1, &(tid.raw), 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    RunIntThread();

    EXIT;
    return ERR_NONE;
}

status_t
UnsetInterrupt(UInt num)
{
    L4_Msg_t msg;
    status_t err;

    L4_Put(&msg, MSG_ROOT_UNSET_INT, 1, (L4_Word_t *)&num, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    return ERR_NONE;
}

L4_ThreadId_t
GetIntrThread()
{
    return _intrThread;
}


