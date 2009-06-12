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
/// @brief  Initial routine for level-2 user tasks
/// @file   Libraries/Level2/src/init.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///

//$Id: init.cpp 386 2008-08-30 05:07:57Z hro $

#include <System.h>
#include <Ipc.h>
#include <Console.h>

extern void _palloc_init(addr_t base);
extern void _malloc_init();

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL 0
#endif // VERBOSE_LEVEL


#define LINE_BUFFER_SIZE    PAGE_SIZE
Console __console;

SystemHelper System(VERBOSE_LEVEL, &__console); 

void
__init_task(addr_t heap_base)
{
    _palloc_init(heap_base);
    _malloc_init();
}

void
__init_console()
{
    __console.Initialize(LINE_BUFFER_SIZE);
}

void
__exit(int stat)
{
    L4_Msg_t    msg;
    L4_Word_t   reg;

    __console.Stop();

    reg = stat;
    L4_Put(&msg, MSG_ROOT_EXIT, 1, &reg, 0, 0);
    Ipc::Send(L4_Pager(), &msg);
    L4_Sleep(L4_Never);
}

