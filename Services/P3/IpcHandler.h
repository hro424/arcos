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
/// @brief  IPC handler of Pel
/// @file   Services/Pel2/IpcHandler.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: IpcHandler.h 384 2008-08-28 18:38:10Z hro $

#ifndef ARC_PEL2_IPC_HANDLER_H
#define ARC_PEL2_IPC_HANDLER_H

#include <Types.h>
#include <l4/types.h>
#include "Loader.h"
#include "Task.h"

class IpcHandler
{
protected:
    Task            *_task;
    Int             _exit_code;
    L4_MapItem_t    _mapregs[(__L4_NUM_MRS - 1) / 3];
    L4_Word_t       _physregs[(__L4_NUM_MRS - 1) / 3];

    stat_t HandleStartThread(L4_Msg_t *msg);
    stat_t HandleCancel(L4_Msg_t* msg);
    stat_t HandlePelRestart(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandlePageFault(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandleAllocate(L4_Msg_t *msg);
    stat_t HandleRelease(L4_Msg_t *msg);
    stat_t HandleExpandHeap(L4_Msg_t *msg);
    stat_t HandleMap(L4_Msg_t *msg);
    stat_t HandleIoMap(L4_Msg_t *msg);
    stat_t HandleCreateThread(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandleDeleteThread(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandleMapBios(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandleMapLFB(L4_ThreadId_t tid, L4_Msg_t *msg);
    stat_t HandleGetNs(L4_Msg_t *msg);
    stat_t HandleExit(L4_Msg_t *msg);
    stat_t HandleInject(L4_ThreadId_t tid, L4_Msg_t* msg);

    stat_t RecoverCheckpoint(L4_ThreadId_t tid, L4_Word_t gen);
    stat_t RecoverLazy();
    stat_t RecoverURB();

    stat_t Recover();

public:
    stat_t HandleIpc(Task *t);
    Int UserExitCode();
};

inline Int
IpcHandler::UserExitCode()
{
    return _exit_code;
}

#endif // ARC_PEL2_IPC_HANDLER_H

