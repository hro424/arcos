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
/// @brief  Task operations protocol handler
///
/// NOTE:
/// The creation of threads and address spaces can only be done by the threads
/// in the privileged address space such as root task, Sigma0, or Sigma1.
/// @see kernel source code (e.g. SYS_THREAD_CONTROL)
///
/// @file   Services/Root/Server0.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///


#include <Debug.h>
#include <Ipc.h>
#include <System.h>
#include <Types.h>
#include "Common.h"
#include "NameService.h"
#include "PageFrameTable.h"
#include "Space.h"
#include "Task.h"
#include "Thread.h"

#include <l4/thread.h>
#include <l4/types.h>
#include <l4/schedule.h>
#include <l4/message.h>

#include "PageAllocator.h"


extern PageFrameTable   MainPft;
extern L4_ThreadId_t    RootPagerId;

NameService      _ns;

///
/// Creates a new address space and its main thread.
///
static stat_t HandleCreateTask(L4_ThreadId_t tid, L4_Msg_t *msg);

///
/// Deletes the specified address space and all the resident threads.
///
static stat_t HandleDeleteTask(L4_Msg_t *msg);

///
/// Creates a new thread in the specified address space.
///
static stat_t HandleCreateThread(L4_Msg_t *msg);

///
/// Deletes the specified thread.
///
static stat_t HandleDeleteThread(L4_ThreadId_t tid, L4_Msg_t *msg);

static stat_t HandleSetInterrupt(L4_Msg_t *msg);
static stat_t HandleUnsetInterrupt(L4_Msg_t *msg);
static stat_t HandlePriority(L4_Msg_t *msg);
static stat_t HandleExecTask(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleExitTask(L4_ThreadId_t tid, L4_Msg_t *msg);
static stat_t HandleKillTask(L4_Msg_t* msg);

static stat_t HandleNsSearch(L4_Msg_t *msg);
static stat_t HandleNsInsert(L4_Msg_t *msg);
static stat_t HandleNsRemove(L4_Msg_t* msg);
static stat_t HandleNsList(L4_Msg_t *msg);

static stat_t HandleRestart(L4_ThreadId_t tid, L4_Msg_t *msg);

///
/// Takes a snapshot of the specified address space.
///
static stat_t HandleSnapshot(L4_ThreadId_t tid, L4_Msg_t *msg);

///
/// Restores the snapshot of the specified address space.
///
static stat_t HandleRestore(L4_ThreadId_t tid, L4_Msg_t *msg);

static stat_t HandleFindAS(L4_ThreadId_t tid, L4_Msg_t* msg);

static stat_t HandleInject(L4_ThreadId_t tid, L4_Msg_t* msg);


void
InitProcMan()
{
    InitializeTaskManagement();
}

void
ProcManMain(void)
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    L4_ThreadId_t   peer;

    System.Print(System.INFO, "Starting Core Service (%.8lX) ...\n",
                 L4_Myself().raw);

    NotifyReady();

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

        L4_MsgStore(tag, &msg);
        switch (L4_MsgLabel(&msg) & MSG_MASK) {
            case MSG_ROOT_NEW_TSK:
                HandleCreateTask(peer, &msg);
                break;
            case MSG_ROOT_DEL_TSK:
                HandleDeleteTask(&msg);
                break;
            case MSG_ROOT_NEW_TH:
                HandleCreateThread(&msg);
                break;
            case MSG_ROOT_DEL_TH:
                HandleDeleteThread(peer, &msg);
                break;
            case MSG_ROOT_SET_INT:
                HandleSetInterrupt(&msg);
                break;
            case MSG_ROOT_UNSET_INT:
                HandleUnsetInterrupt(&msg);
                break;
            case MSG_ROOT_PRIO:
                HandlePriority(&msg);
                break;
            case MSG_ROOT_EXEC:
                HandleExecTask(peer, &msg);
                break;
            case MSG_ROOT_KILL:
                HandleKillTask(&msg);
                break;
            case MSG_ROOT_EXIT:
                HandleExitTask(peer, &msg);
                goto begin;
                break;
            case MSG_ROOT_RESTART: // For experimental use
                HandleRestart(peer, &msg);
                break;
            case MSG_ROOT_SNAPSHOT:
                HandleSnapshot(peer, &msg);
                break;
            case MSG_ROOT_RESTORE:
                HandleRestore(peer, &msg);
                break;
            case MSG_ROOT_INJECT:
                HandleInject(peer, &msg);
                break;
            case MSG_ROOT_FIND_AS:
                HandleFindAS(peer, &msg);
                break;
            case MSG_NS_SEARCH:
                DOUT("ns search from %.8lX\n", peer.raw);
                HandleNsSearch(&msg);
                break;
            case MSG_NS_INSERT:
                HandleNsInsert(&msg);
                break;
            case MSG_NS_REMOVE:
                HandleNsRemove(&msg);
                break;
            case MSG_NS_LIST:
                HandleNsList(&msg);
                break;
            default:
                System.Print(System.WARN,
                             "Server0: Unknown message: %.8lX from %.8lX\n",
                             L4_Label(&msg), peer.raw);
                // check if the partner blocks
                goto begin;
        }
            
        L4_Load(&msg);
        tag = L4_ReplyWait(peer, &peer);
    }
exit:
    FATAL("Server0 is going to be terminated.");
}

///
/// Builds the message for creating a new task.
/// @param tid   the ID of the thread requesting the creation
/// @param msg   the corresponding message to be sent is built here
///
static stat_t
HandleCreateTask(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    stat_t        err;
    Space           *space;
    L4_ThreadId_t   pg;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    pg.raw = L4_Get(msg, 0);

    err = CreateTask(&space, FALSE);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    err = ActivateThread(space->GetRootThread(), L4_Myself(), pg);
    if (err != ERR_NONE) {
        DeleteTask(space);
        return Ipc::ReturnError(msg, err);
    }

    space->SetPager(pg);

    L4_Word_t reg = space->GetRootThread()->Id.raw;
    L4_Put(msg, ERR_NONE, 1, &reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

static stat_t
HandleDeleteTask(L4_Msg_t *msg)
{
    L4_ThreadId_t   tid;
    Space*          space;
    stat_t          err;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    tid.raw = L4_Get(msg, 0);
    if (FindTask(tid, &space) != ERR_NONE) {
        DOUT("find task failed\n");
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    // NOTE: This deletes all the threads running in the space.
    err = DeleteTask(space);
    if (err != ERR_NONE) {
        DOUT("delete task failed\n");
        return Ipc::ReturnError(msg, err);
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleCreateThread(L4_Msg_t *msg)
{
    stat_t        err;
    Space*          space;
    Thread*         thread;
    L4_ThreadId_t   tid;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    //
    // Need space id to create a thead in the "user's" space, not in the
    // "shadow task's" space
    //
    tid.raw = L4_Get(msg, 0);
    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    // Create an inactive thread in the specified space
    // TID is automatically generated.
    err = space->CreateThread(&thread);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    err = ActivateThread(thread, L4_Myself(), space->GetPager());
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    L4_Put(msg, ERR_NONE, 1, &thread->Id.raw, 0, 0);

    EXIT;
    return ERR_NONE;
}

static stat_t
HandleDeleteThread(L4_ThreadId_t pel, L4_Msg_t *msg)
{
    Space           *space;
    stat_t          err;
    Thread          *thread;
    L4_ThreadId_t   tid, sid;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    tid.raw = L4_Get(msg, 0);
    sid.raw = L4_Get(msg, 1);

    if (FindTask(sid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    if (space->GetPager() != pel) {
        return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    if (space->FindThread(tid, &thread) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_THREAD);
    }

    err = DeleteThread(thread);

    EXIT;
    return Ipc::ReturnError(msg, err);
}

static stat_t
HandleSetInterrupt(L4_Msg_t *msg)
{
    L4_ThreadId_t   th;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    th.raw = L4_Get(msg, 0);

    System.Print("Attach interrupt %.8lX to %.8lX\n",
                 L4_GlobalId(L4_Version(th), 1).raw,
                 L4_GlobalId(L4_ThreadNo(th),
                             Thread::TID_INITIAL_VERSION).raw);
    L4_DeassociateInterrupt(L4_GlobalId(L4_Version(th), 1));
    L4_AssociateInterrupt(L4_GlobalId(L4_Version(th), 1),
                          L4_GlobalId(L4_ThreadNo(th),
                                      Thread::TID_INITIAL_VERSION));

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleUnsetInterrupt(L4_Msg_t *msg)
{
    L4_ThreadId_t   th;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    th.raw = L4_Get(msg, 0);
    L4_DeassociateInterrupt(L4_GlobalId(L4_Version(th), 1));

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandlePriority(L4_Msg_t *msg)
{
    L4_ThreadId_t tid;
    L4_Word_t prio;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }
    
    tid.raw = L4_Get(msg, 0);
    prio = L4_Get(msg, 1);

    L4_Set_Priority(tid, prio);

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleExecTask(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t       argc;
    L4_Word_t       argv;
    L4_ThreadId_t   new_tid;
    PageFrame*      frame;
    Space*          space;
    stat_t          err;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    argc = L4_Get(msg, 0);
    argv = L4_Get(msg, 1);

    //
    // Find the page where the argv is allocated.
    //
    if (space->SearchMap(argv & PAGE_MASK, &frame) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    //
    // Convert user's address to root task's address
    //
    argv = MainPft.GetAddress(frame) + (argv & ~PAGE_MASK);

    char** pp = (char**)argv;
    for (L4_Word_t i = 0; i < argc; i++) {
        if (space->SearchMap(reinterpret_cast<L4_Word_t>(pp[i]) & PAGE_MASK,
                             &frame) != ERR_NONE) {
            return Ipc::ReturnError(msg, ERR_NOT_FOUND);
        }
        pp[i] = reinterpret_cast<char*>(MainPft.GetAddress(frame) +
                            (reinterpret_cast<L4_Word_t>(pp[i]) & ~PAGE_MASK));
        DOUT("argv[%u]: '%s'\n", i, pp[i]);
    }

    err = ExecTask(argc, (char**)argv, &new_tid);

    L4_Put(msg, ERR_NONE, 1, &new_tid.raw, 0, 0);

    EXIT;
    return ERR_NONE;
}

static stat_t
HandleKillTask(L4_Msg_t *msg)
{
    L4_ThreadId_t   tid;
    Space*          space;

    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    tid.raw = L4_Get(msg, 0);
    if (FindTask(tid, &space) != ERR_NONE) {
        DOUT("find task failed\n");
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    L4_Word_t   reg = 0;
    L4_Msg_t    m;
    L4_Put(&m, MSG_ROOT_EXIT, 1, &reg, 0, 0);
    Ipc::Send(space->GetPager(), &m);

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}


static stat_t
HandleExitTask(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t       exit_status;
    Space           *space;
    stat_t        err;

    ENTER;

    exit_status = L4_MsgWord(msg, 0);

    if (FindTask(tid, &space) != ERR_NONE) {
        DOUT("error\n");
        return ERR_INVALID_SPACE;
    }

    // NOTE: This deletes all the threads running in the space.
    err = DeleteTask(space);
    if (err != ERR_NONE) {
        DOUT("error\n");
        return err;
    }

    System.Print(System.INFO, "art: handle exit: %.8lX %.8lX\n",
                 tid.raw, exit_status);
    EXIT;
    return ERR_NONE; 

}

static stat_t
HandleNsSearch(L4_Msg_t *msg)
{
    L4_Word_t reg;
    L4_ThreadId_t tid;
    char *name;

    ENTER;

    name = (char *)(&msg->raw[1]);
    DOUT("name to search: %s\n", name);
    tid = _ns.Search(name);
    reg = tid.raw;
    L4_Put(msg, ERR_NONE, 1, &reg, 0, 0);

    EXIT;

    return ERR_NONE;
}

static stat_t
HandleNsInsert(L4_Msg_t *msg)
{
    char *name;
    L4_ThreadId_t tid;
    ENTER;

    tid.raw = msg->raw[1];
    name = (char *)(&msg->raw[2]);
    DOUT("Insert '%s' @ %.8lX\n", name, tid.raw);
    _ns.Insert(name, tid);

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleNsRemove(L4_Msg_t* msg)
{
    char*   name;

    name = (char*)(&msg->raw[1]);
    _ns.Remove(name);
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleNsList(L4_Msg_t* msg)
{
#define MAX_REGS        16
#define MAX_STRLEN      (MAX_REGS * 4)

    L4_Word_t   reg[MAX_REGS + 2];
    NameEntry*  e;
    Space*      s;
    L4_Word_t   i;
    size_t      len;
    Iterator<NameEntry*>& it = _ns.GetList();

    i = L4_Get(msg, 0);
    for (L4_Word_t j = 0; j < i + 1; j++) {
        if (it.HasNext()) {
            e = it.Next();
        }
        else {
            e = 0;
        }
    }

    if (e == 0) {
        return Ipc::ReturnError(msg, ERR_NOT_FOUND);
    }

    FindTask(e->tid, &s);

    reg[0] = e->tid.raw;
    reg[1] = s->GetPager().raw;
    len = strlen(e->name) + 1;
    if (len > MAX_STRLEN) {
        len = MAX_STRLEN;
    }
    memcpy(&reg[2], e->name, len);
    DOUT("%.8lX %.8lX %s\n", reg[0], reg[1], e->name);

    L4_Put(msg, ERR_NONE, (len + 3) / 4 + 2, reg, 0, 0);
    return ERR_NONE;
}

static stat_t
HandleRestart(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_ThreadId_t   target;
    L4_ThreadId_t   pel;
    L4_Word_t       reg;
    Space*          space;
    stat_t        err;
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    target.raw = L4_MsgWord(msg, 0);
    reg = L4_MsgWord(msg, 1);

    if (FindTask(target, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_THREAD);
    }

    pel = space->GetPager();

    if (L4_IsThreadEqual(pel, RootPagerId)) {
        // Pel is specified as restart target.
        return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    // You can't restart yourself.
    if (L4_IsThreadEqual(pel, tid)) {
        return Ipc::ReturnError(msg, ERR_INVALID_RIGHTS);
    }

    System.Print("Root: restart %.8lX, pel %.8lX\n", target.raw, pel.raw);

    L4_Put(msg, MSG_PEL_RESTART, 1, &reg, 0, 0);
    err = Ipc::Send(pel, msg);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_UNKNOWN);
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleSnapshot(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    L4_Word_t ip = L4_MsgWord(msg, 0);
    L4_Word_t sp = L4_MsgWord(msg, 1);

    Space *space;

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    stat_t err = space->Snapshot(ip, sp);
    if (err != 0) {
        Ipc::ReturnError(msg, err);
    }

    EXIT;
    return Ipc::ReturnError(msg, ERR_NONE);
}

static stat_t
HandleRestore(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    L4_Word_t   reg[2];
    Space       *space;
#ifdef SYS_DEBUG
    static L4_Word_t    restore_count = 1;
#endif // SYS_DEBUG
    ENTER;

    if (Ipc::CheckPayload(msg, 0, 1)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    if (FindTask(tid, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    L4_Word_t gen = L4_MsgWord(msg, 0);

    DOUT("Restore[%lu] %lu gen\n", restore_count, gen);
#ifdef SYS_DEBUG
    restore_count++;
#endif // SYS_DEBUG

    stat_t err = space->Restore(gen, &reg[0], &reg[1]);
    if (err != ERR_NONE) {
        return Ipc::ReturnError(msg, err);
    }

    DOUT("ip %.8lX sp %.8lX\n", reg[0], reg[1]);
    L4_MsgPut(msg, 0, 2, reg, 0, 0);
    EXIT;
    return ERR_NONE;
}

static stat_t
HandleInject(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    Space*          space;
    L4_ThreadId_t   target;
    L4_Msg_t        m;
    L4_Word_t       area;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return Ipc::ReturnError(msg, ERR_INVALID_ARGUMENTS);
    }

    target.raw = L4_Get(msg, 0);
    area = L4_Get(msg, 1);

    if (FindTask(target, &space) != ERR_NONE) {
        return Ipc::ReturnError(msg, ERR_INVALID_SPACE);
    }

    L4_Put(&m, MSG_PEL_INJECT, 1, &area, 0, 0);
    return Ipc::Send(space->GetPager(), &m);
}

static stat_t
HandleFindAS(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    Space*          space;
    L4_ThreadId_t   id;

    id.raw = L4_Get(msg, 0);
    if (FindTask(id, &space) == ERR_NONE) {
        L4_Word_t reg = space->GetRootThread()->Id.raw;
        L4_Put(msg, ERR_NONE, 1, &reg, 0, 0);
    }
    else {
        L4_Put(msg, ERR_NOT_FOUND, 0, 0, 0, 0);
    }
    return ERR_NONE;
}

