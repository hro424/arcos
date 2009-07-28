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
/// @file   Libraries/Arc/include/Server.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: Server.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_SERVER_TEMPLATE_H_
#define ARC_SERVER_TEMPLATE_H_

#include <Debug.h>
#include <List.h>
#include <l4/message.h>
#include <l4/types.h>
#include <Ipc.h>

class BasicServer
{
protected:
    virtual stat_t IpcHandler(const L4_ThreadId_t& tid, L4_Msg_t& msg);
public:
    stat_t Run();
    virtual const char* const Name() = 0;
    virtual stat_t Initialize(Int argc, char* argv[]) = 0;
    virtual stat_t Exit();
};

#define ARC_SERVER(CLASS)                                   \
    int server_main(int argc, char* argv[], int state)      \
    {                                                       \
        stat_t  err;                                        \
        CLASS   server;                                     \
        System.Print("Starting %s (%.8lX) ...\n",           \
                     argv[0], L4_Myself().raw);             \
        err = server.Initialize(argc, argv);                \
        System.Print("'%s' (%.8lX) initialized.\n",         \
                     argv[0], L4_Myself().raw);             \
        if (err != ERR_NONE) {                              \
            return static_cast<int>(err);                   \
        }                                                   \
        err = server.Run();                                 \
        return static_cast<int>(err);                       \
    }

#define SERVER_HANDLER_BEGIN(CLASS)                                 \
    stat_t CLASS::IpcHandler(const L4_ThreadId_t& tid, L4_Msg_t& msg) {     \
        stat_t err = ERR_UNKNOWN;                                           \
        switch (L4_Label(L4_MsgTag(&msg))) {

#define SERVER_HANDLER_CONNECT(MSGTYPE, HANDLER)    \
            case MSGTYPE: err = HANDLER(tid, msg); break;

#define SERVER_HANDLER_END                                                  \
            default:                                                        \
                System.Print(System.WARN,                                   \
                             "%s: Unknown message %lX from %.8lX\n",        \
                             Name(), L4_Label(L4_MsgTag(&msg)), tid.raw);   \
                err = ERR_NOT_FOUND;                                        \
                break;                                                      \
        }                                                                   \
        return err;                                                         \
    }

class SelfHealingServer : public BasicServer
{
public:
    stat_t Run(int state);
    virtual stat_t Recover(Int argc, char* argv[]) { return ERR_NONE; };
};

#define IS_PERSISTENT __attribute__ ((section(".pdata")))

#define ARC_SH_SERVER(CLASS)                                \
    int server_main(int argc, char* argv[], int state) {    \
        stat_t              err = ERR_UNKNOWN;              \
        CLASS               instance;                       \
        System.Print("Starting %s (%.8lX) ...\n",           \
                     argv[0], L4_Myself().raw);             \
        SelfHealingServer&  server = instance;              \
        switch (state) {                                    \
            case 0:                                         \
                err = server.Initialize(argc, argv);        \
                System.Print("'%s' (%.8lX) initialized.\n", \
                             argv[0], L4_Myself().raw);     \
                break;                                      \
            case 1:                                         \
            case 2:                                         \
                err = server.Recover(argc, argv);           \
                System.Print("'%s' (%.8lX) recovered.\n",   \
                             argv[0], L4_Myself().raw);     \
                break;                                      \
            default:                                        \
                break;                                      \
        }                                                   \
        if (err != ERR_NONE) {                              \
            return static_cast<int>(err);                   \
        }                                                   \
        server.Run(state);                                  \
        err = server.Exit();                                \
        return static_cast<int>(err);                       \
    }

struct SessionClient
{
    L4_ThreadId_t   tid;
    addr_t          base;
    size_t          size;

    SessionClient(L4_ThreadId_t t, addr_t b, size_t s)
        : tid(t), base(b), size(s) {}

    virtual ~SessionClient() {}
};

class SessionServer : public BasicServer
{
protected:
    List<SessionClient*>        _clients;

    virtual void Register(const L4_ThreadId_t& tid, addr_t base, size_t size);
    virtual void Deregister(SessionClient* c);
    SessionClient* Search(const L4_ThreadId_t& tid, addr_t base);

    virtual stat_t IpcHandler(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleDisconnect(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg);

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
    SessionServer() {}
    virtual ~SessionServer() {}
};

#endif // ARC_SERVER_H

