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
/// @brief  Fault injector
/// @file   Applications/Injector/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

#include <Ipc.h>
#include <FaultInjection.h>
#include <NameService.h>
#include <System.h>

#define MS(T)       (T * 1000)
#define S(T)        (T * 1000000)

int
main(int argc, char* argv[])
{
    L4_ThreadId_t   target;
    L4_Msg_t        msg;
    stat_t          err;

    if (argc < 2) {
        return 1;
    }

    // Get the target ID
    err = NameService::Get(argv[1], &target);

    L4_Word_t reg[2];
    reg[0] = target.raw;
    reg[1] = FI_TEXT_AREA;

    System.Print("%.8lX starts fault injection to '%s' %.8lX.\n",
                 L4_Myself().raw, argv[1], target.raw);
    for (int i = 0; i < 10; i++) {
        // Sleep
        L4_Sleep(L4_TimePeriod(MS(500UL)));

        // Send restart request
        L4_Put(&msg, MSG_ROOT_INJECT, 2, reg, 0, 0);
        Ipc::Call(L4_Pager(), &msg, &msg);
    }

    return 0;
}

