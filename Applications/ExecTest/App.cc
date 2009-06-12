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
/// @file   Applications/ExecTest/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @date   2007
///

// $Id: App.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/ipc.h>
#include <arc/protocol.h>
#include <arc/status.h>
#include <arc/string.h>
#include <arc/system.h>
#include <l4/message.h>
#include <l4/types.h>

#include <stdio.h>

int
main()
{
    static const char  *path = "/test2/test21/hello";
    L4_Msg_t    msg;
    L4_Word_t   reg[2];
    status_t    err;

    printf("-- Program exec test --\n");

    // ART is sharing the address. You don't have to put the string in the
    // registers.
    reg[0] = (L4_Word_t)path;
    reg[1] = strlen(path) + 1;

    L4_MsgPut(&msg, MSG_ROOT_EXEC, 2, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return 1;
    }

    printf("-- Program exec test end --\n");
    L4_Sleep(L4_Never);
    return 0;
}

