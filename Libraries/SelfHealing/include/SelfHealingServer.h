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
/// @brief  Self healing server template
/// @file   Libraries/Persistent/include/SelfHealingServer.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  May 2008
///

//$Id: SelfHealingServer.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_SELF_HEALING_SERVER_H
#define ARC_SELF_HEALING_SERVER_H

#include <Server.h>

struct SessionControlBlock
{
    L4_ThreadId_t   tid;
    addr_t          base;
    size_t          size;
    word_t          data;
};


class SelfHealingSessionServer : public SelfHealingServer
{
protected:
    void Register(const L4_ThreadId_t& tid, addr_t base, size_t size);
    void Deregister(const L4_ThreadId_t& tid, addr_t base);
    void Deregister(SessionControlBlock* c);
    SessionControlBlock* Search(const L4_ThreadId_t& tid, addr_t base);

    virtual stat_t IpcHandler(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleDisconnect(const L4_ThreadId_t& tid, L4_Msg_t& msg);

    virtual stat_t HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Clear(&msg);
        return ERR_NONE;
    }

    virtual stat_t HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Clear(&msg);
        return ERR_NONE;
    }

    virtual stat_t HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Clear(&msg);
        return ERR_NONE;
    }

    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Clear(&msg);
        return ERR_NONE;
    }

    L4_ThreadId_t FindSpace(L4_ThreadId_t t)
    {
        L4_Msg_t        msg;
        L4_ThreadId_t   sid;
        L4_Put(&msg, MSG_ROOT_FIND_AS, 1, &t.raw, 0, 0);
        Ipc::Call(L4_Pager(), &msg, &msg);
        sid.raw = L4_Get(&msg, 0);
        return sid;
    }

public:
    static const int NUM_CLIENTS = 10;

    SelfHealingSessionServer() {}
    virtual ~SelfHealingSessionServer() {}
};

#define ARC_SHS_SERVER(CLASS)                                       \
    extern void _shmalloc_init(addr_t base, size_t size);           \
    int server_main(int argc, char* argv[], int state) {            \
        stat_t              err = ERR_UNKNOWN;                      \
        CLASS               instance;                               \
        SelfHealingServer&  server = instance;                      \
        System.Print("Starting %s (%.8lX) stat:%d ...\n",           \
                     argv[0], L4_Myself().raw, state);              \
        switch (state) {                                            \
            case 0:                                                 \
                _shmalloc_init(VirtLayout::SHM_START, 0x100000);    \
                err = server.Initialize(argc, argv);                \
                System.Print("'%s' (%.8lX) initialized.\n",         \
                             argv[0], L4_Myself().raw);             \
                break;                                              \
            case 1:                                                 \
            case 2:                                                 \
                err = server.Recover(argc, argv);                   \
                System.Print("'%s' (%.8lX) recovered.\n",           \
                             argv[0], L4_Myself().raw);             \
                break;                                              \
            default:                                                \
                break;                                              \
        }                                                           \
        if (err != ERR_NONE) {                                      \
            return static_cast<int>(err);                           \
        }                                                           \
        server.Run(state);                                          \
        err = server.Exit();                                        \
        return static_cast<int>(err);                               \
    }

#endif // ARC_SELF_HEALING_SERVER_H

