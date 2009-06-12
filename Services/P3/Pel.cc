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
/// @brief  Pager and Loader, ver. 2
/// @file   Services/Pel2/Pel.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Pel.cc 410 2008-09-08 12:00:15Z hro $

#include <Ipc.h>
#include <Types.h>
#include <LoadInfo.h>
#include <TaskMap.h>
#include <sys/Config.h>
#include <l4/types.h>
#include "IpcHandler.h"
#include "Loader.h"
#include "Pel.h"

extern L4_ThreadId_t __root_task();
extern void *__get_load_info();

L4_ThreadId_t
Pel::RootTask()
{
    return __root_task();
}

const LoadInfo *
Pel::GetLoadInfo()
{
    return (const LoadInfo *)__get_load_info();
}

#define STRING_WORD_LEN(str)    \
    (strlen(str) + 1 + sizeof(L4_Word_t) - 1) / sizeof(L4_Word_t);

L4_ThreadId_t
Pel::GetFs(const char* fs)
{
    L4_ThreadId_t   tid;
    L4_Msg_t        msg;
    L4_Word_t       len;
    stat_t          err;

    len = STRING_WORD_LEN(fs);
    if (len >= __L4_NUM_MRS) {
        len = __L4_NUM_MRS - 1;
    }

    L4_Put(&msg, MSG_NS_SEARCH, len, (L4_Word_t *)fs, 0, 0);

    err = Ipc::Call(RootTask(), &msg, &msg);
    if (err != ERR_NONE) {
        return L4_nilthread;
    }

    tid.raw = L4_Get(&msg, 0);

    return tid;
}

