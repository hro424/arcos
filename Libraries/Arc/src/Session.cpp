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
/// @file   Libraries/Arc/src/Session.cpp
/// @brief  Implementation of shared memory regions between tasks
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Session.cpp 429 2008-11-01 02:24:02Z hro $

//#define SYS_DEBUG
//#define SYS_DEBUG_CALL

#include <Debug.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <Mutex.h>
#include <PageAllocator.h>
#include <Session.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>
#include <l4/message.h>
#include <l4/types.h>

const size_t  Session::MAX_REGISTERS = 60;

void
Session::Disconnect(L4_ThreadId_t tid, addr_t dest)
{
    L4_Msg_t msg;
    L4_Put(&msg, MSG_SESSION_DISCONNECT, 1, &dest, 0, 0);
    if (Ipc::Call(tid, &msg, &msg) != ERR_NONE) {
        //XXX: PANIC
    }
}

stat_t
Session::Connect(L4_ThreadId_t peer, L4_Word_t* regs, size_t count)
{
    ENTER;
    stat_t      err;
    L4_Msg_t    msg;

    DOUT("establish a sesson with %.8lX\n", peer.raw);
    L4_Clear(&msg);
    L4_Set_Label(&msg, MSG_SESSION_CONNECT);
    err = Ipc::Call(peer, &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }
    _dest = L4_Get(&msg, 0);
    _size = L4_Get(&msg, 1);

    DOUT("dest: %.8lX, size: %u\n", _dest, _size);
    _shm = palloc_shm(_size, peer, L4_ReadWriteOnly);
    if (_shm == 0) {
        Disconnect(peer, _dest);
        return ERR_OUT_OF_MEMORY;
    }

    DOUT("shm: %.8lX\n", _shm);
    // Map the given pages to this address space.
    for (size_t i = 0; i < _size; i++) {
        err = Pager.Map(_shm + i * PAGE_SIZE, L4_ReadWriteOnly,
                        _dest + i * PAGE_SIZE, peer);
        DOUT("%u\n", err);
        if (err != ERR_NONE) {
            for (size_t j = 0; j < i; j++) {
                err = Pager.Release(_shm + j * PAGE_SIZE);
                if (err != ERR_NONE) {
                    //XXX: PANIC
                    FATAL("page release");
                }
            }
            Disconnect(peer, _dest);
            pfree(_shm, _size);
            _shm = 0;
            return err;
        }
    }

    _peer = peer;

    if (regs != 0) {
        size_t len = L4_UntypedWords(msg.tag);
        if (len < count) {
            count = len;
        }
        for (size_t i = 0; i < count; i++) {
            regs[i] = L4_Get(&msg, i + 1);
        }
    }

    EXIT;
    return ERR_NONE;
}

Session::~Session()
{
    ENTER;
    if (IsConnected()) {
        Disconnect(_peer, _dest);
        _peer = L4_nilthread;
    }
    if (_shm != 0) {
        pfree(_shm, _size);
    }
    EXIT;
}

stat_t
Session::Xfer(L4_Word_t label, L4_Word_t* regs, size_t count)
{
    L4_Msg_t msg;

    L4_Clear(&msg);
    L4_Set_Label(&msg, label);
    L4_Append(&msg, _dest);
    if (regs != 0) {
        if (count > MAX_REGISTERS - 1) {
            count = MAX_REGISTERS - 1;
        }
        for (size_t i = 0; i < count; i++) {
            L4_Append(&msg, regs[i]);
        }
    }
    return Ipc::Call(_peer, &msg, &msg);
}

///
/// @param label    the message label
/// @param sregs    the sending registers
/// @param scount   the count of sending registers
/// @param rregs    the receiving registers
/// @param rcount   the count of receiving registers
///
stat_t
Session::Xfer(L4_Word_t label, L4_Word_t* sregs, size_t scount,
              L4_Word_t* rregs, size_t rcount)
{
    L4_Msg_t    msg;
    stat_t      err;

    L4_Clear(&msg);
    L4_Set_Label(&msg, label);
    L4_Append(&msg, _dest);
    if (sregs != 0) {
        if (scount > MAX_REGISTERS - 1) {
            scount = MAX_REGISTERS - 1;
        }
        for (size_t i = 0; i < scount; i++) {
            L4_Append(&msg, sregs[i]);
        }
    }
    err = Ipc::Call(_peer, &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    if (rregs != 0) {
        size_t len = L4_UntypedWords(msg.tag);
        if (len < rcount) {
            rcount = len;
        }
        for (size_t i = 0; i < rcount; i++) {
            rregs[i] = L4_Get(&msg, i + 1);
        }
    }
    return ERR_NONE;
}

stat_t
Session::PutAsync(L4_Word_t* regs, size_t count)
{
    L4_Msg_t msg;

    L4_Clear(&msg);
    L4_Set_Label(&msg, MSG_SESSION_PUT_ASYNC);
    L4_Append(&msg, _dest);
    if (count > MAX_REGISTERS - 1) {
        count = MAX_REGISTERS - 1;
    }
    for (size_t i = 0; i < count; i++) {
        L4_Append(&msg, regs[i]);
    }
    return Ipc::Send(_peer, &msg);
}

