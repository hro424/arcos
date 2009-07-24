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
/// @brief  Keyboard server
/// @file   Services/Devices/HID/Keyboard/server.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: server.cc 429 2008-11-01 02:24:02Z hro $

#include <Debug.h>
#include <Interrupt.h>
#include <Ipc.h>
#include <List.h>
#include <System.h>
#include <Types.h>
#include <arc/IO.h>
#include <l4/types.h>
#include <l4/kdebug.h>

///
/// Handles keyboard input and delivers it to clients.
///
class KeyAcceptor : public InterruptHandler
{
private:
    ///
    /// Threads that are interested in key inputs.
    ///
    List<L4_ThreadId_t>     _listeners;

public:
    KeyAcceptor() {}

    virtual ~KeyAcceptor()
    {
        L4_Msg_t                    msg;
        Iterator<L4_ThreadId_t>&    it = _listeners.GetIterator();

        L4_Put(&msg, MSG_EVENT_TERMINATE, 0, 0, 0, 0);
        while (it.HasNext()) {
            Ipc::Send(it.Next(), &msg);
        }
    }

    virtual void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg);

    void AddListener(L4_ThreadId_t tid) { _listeners.Add(tid); }

    void DelListener(L4_ThreadId_t tid) { _listeners.Remove(tid); }
};

void
KeyAcceptor::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    ENTER;

    L4_Word_t                   data;
    L4_Msg_t                    event;
    Iterator<L4_ThreadId_t>&    it = _listeners.GetIterator();

    data = inb(0x60);
    DOUT("keycode: %lu (0x%.lX) -> %u listeners\n",
         data, data, _listeners.Length());
    if (data == 9) {
        L4_KDB_Enter("enter debug");
        return;
    }

    L4_Put(&event, MSG_EVENT_NOTIFY, 1, &data, 0, 0);
    while (it.HasNext()) {
        Ipc::Send(it.Next(), &event);
    }
    EXIT;
}


static void
HandleIpc(KeyAcceptor *acceptor)
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    L4_ThreadId_t   peer;
    stat_t        stat = ERR_NONE;

begin:
    tag = L4_Wait(&peer);
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

        L4_Store(tag, &msg);
        switch (L4_Label(&msg) & MSG_MASK) {
            case MSG_EVENT_CONNECT:
            {
                L4_ThreadId_t evt_src = acceptor->Id();
                acceptor->AddListener(peer);
                stat = ERR_NONE;
                L4_Put(&msg, ERR_NONE, 1, &evt_src.raw, 0, 0);
                break;
            }
            case MSG_EVENT_DISCONNECT:
                acceptor->DelListener(peer);
                stat = ERR_NONE;
                L4_Clear(&msg);
                break;
            default:
                System.Print(System.WARN,
                             "KB: unknown message %lu from %.8lX\n",
                             L4_Label(&msg), peer.raw);
                BREAK("kb err");
                stat = (stat_t)0;
                break;
        }

        if (stat != ERR_NONE) {
            System.Print(System.ERROR, "KB error: %s\n", stat2msg[stat]);
            BREAK("kb err");
            goto exit;
        }

        L4_Load(&msg);
        tag = L4_ReplyWait(peer, &peer);
    }
exit:
    return;
}

int
server_main(int a, char* b[], int c)
{
    System.Print("Starting keyboard server\n");
    static const UInt IRQ_KB = 1;
    InterruptManager *im = InterruptManager::Instance();
    KeyAcceptor *acceptor = new KeyAcceptor;

    im->Register(acceptor, IRQ_KB);
    System.Print("keyboard server initialized.\n");
    HandleIpc(acceptor);
    im->Deregister(IRQ_KB);
    delete acceptor;

    return 0;
}

