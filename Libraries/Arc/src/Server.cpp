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
/// @brief  Server template
/// @file   Libraries/Arc/src/Server.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///


#include <Debug.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include <Server.h>
#include <Session.h>
#include <System.h>
#include <Types.h>
#include <l4/ipc.h>


stat_t
BasicServer::Run()
{
    L4_ThreadId_t   tid;
    L4_MsgTag_t     tag;
    L4_Msg_t        msg;
    stat_t          err = ERR_UNKNOWN;
   
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
                    err = Ipc::ErrorCode();
                    goto exit;
                case ERR_IPC_OVERFLOW:
                default:
                    goto begin;
            }
        }

        L4_Store(tag, &msg);
        err = IpcHandler(tid, msg);
        if (err != ERR_NONE) {
            System.Print(System.WARN,
                         "%s: Error processing message %lx from %.8lX\n",
                         Name(), L4_Label(tag), tid.raw);
            break;
        }
        //DOUT("%.8lX\n", msg.tag);
        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }
exit:
    return err;
}

// The last client
static L4_ThreadId_t _sh_tid IS_PERSISTENT;
// The last message
static L4_Msg_t _sh_msg IS_PERSISTENT;

/*
stat_t
SelfHealingServer::Handle()
{
    stat_t err = IpcHandler(_sh_tid, _sh_msg);
    if (err != ERR_NONE) {
        System.Print(System.WARN,
                     "%s: Error processing message %lx from %.8lX\n",
                     Name(), L4_Label(tag), _sh_tid.raw);
        break;
    }
}
*/

stat_t
SelfHealingServer::Run(int state)
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    stat_t          err = ERR_UNKNOWN;
   
    if (state == 1) {
        goto restore;
    }

begin:
    L4_Put(&msg, MSG_PEL_READY, 0, 0, 0, 0);
    Ipc::Send(L4_Pager(), &msg);
    tag = L4_Wait(&_sh_tid);
    
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
                    goto begin;
            }
        }

        L4_Store(tag, &_sh_msg);
restore:
        err = IpcHandler(_sh_tid, _sh_msg);
        if (err != ERR_NONE) {
            System.Print(System.WARN,
                         "%s: Error processing message %lx from %.8lX\n",
                         Name(), L4_Label(tag), _sh_tid.raw);
            break;
        }
        L4_Load(&_sh_msg);
        tag = L4_ReplyWait(_sh_tid, &_sh_tid);
    }
exit:
    return err;
}

SessionClient*
SessionServer::Search(const L4_ThreadId_t& tid, addr_t base)
{
    L4_ThreadId_t   sid = FindSpace(tid);
    Iterator<SessionClient*>& it = _clients.GetIterator();
    while (it.HasNext()) {
        SessionClient* c = it.Next();
        if (L4_IsThreadEqual(c->tid, sid) && c->base == base) {
            return c;
        }
    }
    return 0;
}

stat_t
SessionServer::HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    ENTER;
    L4_Word_t   reg[2];
    addr_t      shm = palloc(Session::DEFAULT_SHM_PAGES);

    if (shm == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    for (UInt i = 0; i < Session::DEFAULT_SHM_PAGES; i++) {
        Pager.Release(shm + i * PAGE_SIZE);
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
SessionServer::HandleDisconnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    SessionClient*  c;
    addr_t          base;

    ENTER;
    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    pfree(base, c->size);
    Deregister(c);
    L4_Clear(&msg);
    EXIT;
    return ERR_NONE;
}

SERVER_HANDLER_BEGIN(SessionServer)
SERVER_HANDLER_CONNECT(MSG_SESSION_CONNECT, HandleConnect)
SERVER_HANDLER_CONNECT(MSG_SESSION_DISCONNECT, HandleDisconnect)
SERVER_HANDLER_CONNECT(MSG_SESSION_BEGIN, HandleBegin)
SERVER_HANDLER_CONNECT(MSG_SESSION_END, HandleEnd)
SERVER_HANDLER_CONNECT(MSG_SESSION_PUT, HandlePut)
SERVER_HANDLER_CONNECT(MSG_SESSION_GET, HandleGet)
SERVER_HANDLER_END


