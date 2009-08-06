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
/// @brief  Self-healing server template
/// @file   Libraries/Persistent/src/Server.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

#define SYS_DEBUG
//#define SYS_DEBUG_CALL

#include <Debug.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <PersistentPageAllocator.h>
#include <SelfHealingServer.h>
#include <Server.h>
#include <Session.h>
#include <System.h>
#include <Types.h>
#include <l4/ipc.h>

#define SCB_MAXLEN  20
static SessionControlBlock  __scb[SCB_MAXLEN] IS_PERSISTENT;
// Initialize with 0
static Int                  __scb_length IS_PERSISTENT = 0;

void
SelfHealingSessionServer::Register(const L4_ThreadId_t& tid, addr_t base,
                                   size_t size)
{
    L4_ThreadId_t   sid = FindSpace(tid);
    if (__scb_length < SCB_MAXLEN) {
        __scb_length++;
        __scb[__scb_length - 1].tid = sid;
        __scb[__scb_length - 1].base = base;
        __scb[__scb_length - 1].size = size;
        __scb[__scb_length - 1].data = -1;
    }
}

void
SelfHealingSessionServer::Deregister(const L4_ThreadId_t& tid, addr_t base)
{
    if (__scb_length == 0) { 
        return;
    }

    L4_ThreadId_t   sid = FindSpace(tid);
    // Overwrite the removing target with the last valid element of the array.
    for (int i = 0; i < __scb_length - 1; i++) {
        if (L4_IsThreadEqual(__scb[i].tid, sid) &&
            __scb[i].base == base) {
            __scb[i] = __scb[__scb_length - 1];
            break;
        }
    }
    __scb_length--;
}

void
SelfHealingSessionServer::Deregister(SessionControlBlock* c)
{
    // Overwrite the removing target with the last valid element of the array.
    if (__scb_length > 1) {
        *c = __scb[__scb_length - 1];
    }
    __scb_length--;
}

SessionControlBlock*
SelfHealingSessionServer::Search(const L4_ThreadId_t& tid, addr_t base)
{
    L4_ThreadId_t   sid = FindSpace(tid);
    for (int i = 0; i < __scb_length; i++) {
        if (L4_IsThreadEqual(__scb[i].tid, sid) &&
            __scb[i].base == base) {
            return &__scb[i];
        }
    }

    return 0;
}

stat_t
SelfHealingSessionServer::HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    ENTER;
    L4_Word_t   reg[2];
    addr_t      shm = shmalloc(Session::DEFAULT_SHM_PAGES);

    DOUT("shm allocate @ %.8lX\n", shm);

    if (shm == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    //TODO: Hack: Related to a problem in PersistentPageAllocator.cpp
    for (UInt i = 0; i < Session::DEFAULT_SHM_PAGES; i++) {
        Pager.Release(shm + i * PAGE_SIZE);
    }

    for (UInt i = 0; i < Session::DEFAULT_SHM_PAGES; i++) {
        Pager.Reserve(shm + i * PAGE_SIZE, tid, L4_ReadWriteOnly);
    }

    reg[0] = shm;
    reg[1] = Session::DEFAULT_SHM_PAGES;
    L4_Put(&msg, 0, 2, reg, 0, 0);
    Register(tid, reg[0], reg[1]);

    EXIT;
    return ERR_NONE;
}

stat_t
SelfHealingSessionServer::HandleDisconnect(const L4_ThreadId_t& tid,
                                           L4_Msg_t& msg)
{
    SessionControlBlock*    c;
    addr_t                  base;

    ENTER;

    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    if (c == 0) {
        return ERR_NOT_FOUND;
    }

    DOUT("shm release @ %.8lX\n", base);
    shmfree(base, c->size);
    Deregister(c);
    L4_Clear(&msg);
    EXIT;
    return ERR_NONE;
}

stat_t
SelfHealingSessionServer::HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    L4_Clear(&msg);
    return ERR_NONE;
}

stat_t
SelfHealingSessionServer::HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    L4_Clear(&msg);
    return ERR_NONE;
}

stat_t
SelfHealingSessionServer::HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    L4_Clear(&msg);
    return ERR_NONE;
}

stat_t
SelfHealingSessionServer::HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    System.Print("wrong handler\n");
    L4_Clear(&msg);
    return ERR_NONE;
}

SERVER_HANDLER_BEGIN(SelfHealingSessionServer)
SERVER_HANDLER_CONNECT(MSG_SESSION_CONNECT, HandleConnect)
SERVER_HANDLER_CONNECT(MSG_SESSION_DISCONNECT, HandleDisconnect)
SERVER_HANDLER_CONNECT(MSG_SESSION_BEGIN, HandleBegin)
SERVER_HANDLER_CONNECT(MSG_SESSION_END, HandleEnd)
SERVER_HANDLER_CONNECT(MSG_SESSION_PUT, HandlePut)
SERVER_HANDLER_CONNECT(MSG_SESSION_GET, HandleGet)
SERVER_HANDLER_END

