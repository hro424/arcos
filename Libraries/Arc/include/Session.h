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
/// @file   Include/arc/session.h
/// @brief  Communication medium between tasks
/// @date   2007
///

//$Id: Session.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_SESSION_H
#define ARC_SESSION_H

#include <Ipc.h>
#include <Types.h>
#include <l4/types.h>
#include <l4/message.h>
#include <sys/Config.h>

class Session
{
protected:
    static const size_t MAX_REGISTERS;

    ///
    /// The server side of the session
    ///
    L4_ThreadId_t   _peer;

    ///
    /// The base address of the shared memory in the peer.
    ///
    addr_t          _dest;

    ///
    /// The count of pages of the shared memory
    ///
    size_t          _size;

    ///
    /// The base address of the shared memory at this side.
    ///
    addr_t          _shm;

    stat_t Xfer(L4_Word_t label, L4_Word_t* regs, size_t count);

    stat_t Xfer(L4_Word_t label, L4_Word_t* sregs, size_t scount,
                L4_Word_t* rregs, size_t rcount);

    void Disconnect(L4_ThreadId_t tid, addr_t dest);

public:
    static const size_t DEFAULT_SHM_PAGES = 2;

    Session() : _peer(L4_nilthread), _shm(0) {}

    ///
    /// Destroys the session.
    ///
    virtual ~Session();

    ///
    /// Establishes a session with the peer.
    ///
    stat_t Connect(L4_ThreadId_t peer, L4_Word_t* recv_regs, size_t count);

    stat_t Connect(L4_ThreadId_t peer) { return Connect(peer, 0, 0); }

    virtual Bool IsConnected()
    { return L4_IsThreadNotEqual(_peer, L4_nilthread); }

    virtual size_t Size() { return _size * PAGE_SIZE; }

    ///
    /// Obtains the base address of the shared memory.
    ///
    virtual addr_t GetBaseAddress() { return _shm; }

    virtual stat_t Begin(L4_Word_t* send_regs = 0, size_t count = 0)
    { return Xfer(MSG_SESSION_BEGIN, send_regs, count); }

    virtual stat_t Begin(L4_Word_t* send_regs, size_t scount,
                         L4_Word_t* recv_regs, size_t rcount)
    { return Xfer(MSG_SESSION_BEGIN, send_regs, scount, recv_regs, rcount); }

    virtual stat_t End(L4_Word_t* send_regs = 0, size_t count = 0)
    { return Xfer(MSG_SESSION_END, send_regs, count); }

    virtual stat_t Put(L4_Word_t* send_regs, size_t scount,
                       L4_Word_t* recv_regs, size_t rcount)
    { return Xfer(MSG_SESSION_PUT, send_regs, scount, recv_regs, rcount); }

    ///
    /// Notifies the peer that the shared memory is updated.  Waits for a
    /// peer's reply.
    ///
    virtual stat_t Put(L4_Word_t* send_regs, size_t count)
    { return Xfer(MSG_SESSION_PUT, send_regs, count); }

    ///
    /// Notifies the peer that the shared memory is updated.
    ///
    virtual stat_t PutAsync(L4_Word_t* send_regs, size_t count);

    virtual stat_t Get(L4_Word_t* send_regs, size_t scount,
                       L4_Word_t* recv_regs, size_t rcount)
    { return Xfer(MSG_SESSION_GET, send_regs, scount, recv_regs, rcount); }

    ///
    /// Waits for the peer updates the shared memory.
    ///
    virtual stat_t Get(L4_Word_t* send_regs, size_t count)
    { return Xfer(MSG_SESSION_GET, send_regs, count); }
};

#endif // ARC_SESSION_H

