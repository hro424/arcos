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
/// @file   Include/system/ipc.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ipc.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_IPC_H
#define ARC_IPC_H

#include <Protocol.h>
#include <Types.h>
#include <l4/message.h>
#include <l4/ipc.h>
#include <l4/types.h>


class Ipc
{
private:
    static const UInt ERR_IPC_MASK = 0x7;

public:

    ///
    /// Obtains the error code of the last system call.
    ///
    static stat_t ErrorCode()
    {
        return static_cast<stat_t>(((L4_ErrorCode() >> 1) & ERR_IPC_MASK)
                                   + ERR_OFFSET_IPC);
    }

    ///
    /// Makes an IPC call.  Blocks until it receives a reply from the
    /// destination.
    ///
    /// @param dest         the destination thread
    /// @param out          the message to be sent
    /// @param in           the message received
    ///
    static stat_t Call(L4_ThreadId_t dest, L4_Msg_t* out, L4_Msg_t* in)
    {
        L4_MsgTag_t tag;

        L4_Load(out);
        tag = L4_Call(dest);
        if (L4_IpcFailed(tag)) {
            return Ipc::ErrorCode();
        }
        L4_Store(tag, in);
        return static_cast<stat_t>(L4_Label(in));
    }

    ///
    /// Makes an IPC call.  Blocks until it receives a reply from the
    /// destination or runs out of time.
    ///
    /// @param dest         the destination thread
    /// @param timeout      the timeout value
    /// @param out          the message to be sent
    /// @param in           the message received
    ///
    static stat_t Call(L4_ThreadId_t dest, L4_Time_t timeout,
                       L4_Msg_t* out, L4_Msg_t* in)
    {
        L4_MsgTag_t tag;

        L4_Load(out);
        tag = L4_Call(dest, timeout, timeout);
        if (L4_IpcFailed(tag)) {
            return Ipc::ErrorCode();
        }
        L4_Store(tag, in);
        return static_cast<stat_t>(L4_MsgLabel(in));
    }

    ///
    /// Makes a one-way IPC.
    ///
    /// @param dest         the destination thread
    /// @param out          the message to be sent
    ///
    static stat_t Send(L4_ThreadId_t dest, L4_Msg_t* out)
    {
        L4_MsgTag_t tag;
        L4_Load(out);
        tag = L4_Send(dest);
        if (L4_IpcFailed(tag)) {
            return Ipc::ErrorCode();
        }
        return ERR_NONE;
    }

    ///
    /// Checks the contents of the message.  Refer the L4 manual for the
    /// organizatino of an IPC message.
    ///
    /// @param msg          the message to be checked
    /// @param typed        the number of typed registers
    /// @param untyped      the number of untyped registers
    ///
    static Bool CheckPayload(L4_Msg_t* msg, size_t typed, size_t untyped)
    {
        L4_MsgTag_t tag = L4_MsgTag(msg);
        return (L4_TypedWords(tag) != typed || L4_UntypedWords(tag) != untyped);
    }

    ///
    /// Creates an error IPC message.
    ///
    /// @param msg          the message to be filled
    /// @param error        the error code
    ///
    static stat_t ReturnError(L4_Msg_t* msg, stat_t error)
    {
        L4_Put(msg, error, 0, (L4_Word_t *)0, 0, (void *)0);
        return error;
    }
};

#endif // ARC_IPC_H

