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
/// @brief  Debug I/O
/// @file   Libraries/serv/Core/DebugStream.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: DebugStream.cpp 375 2008-08-08 07:53:30Z hro $

#include <DebugStream.h>
#include <Types.h>
#include <l4/kdebug.h>

DebugStream __dstream;

DebugStream::DebugStream() {}

DebugStream::~DebugStream() {}

void
DebugStream::Lock()
{
}

void
DebugStream::Unlock()
{
}

void
DebugStream::Write(Int c)
{
    L4_KDB_PrintChar(c);
}

stat_t
DebugStream::Write(const void* const buf, size_t count, size_t* wsize)
{
    const char* const ptr = static_cast<const char* const>(buf);
    size_t i;
    for (i = 0; i < count; ++i) {
        L4_KDB_PrintChar(ptr[i]);
    }
    *wsize = i;
    return ERR_NONE;
}

Int
DebugStream::Read()
{
    return L4_KDB_ReadChar_Blocked();
}

stat_t
DebugStream::Read(void* buf, size_t count, size_t* rsize)
{
    char* ptr = static_cast<char*>(buf);
    size_t i;
    for (i = 0; i < count; ++i) {
        ptr[i] = L4_KDB_ReadChar_Blocked();
    }
    *rsize = i;
    return ERR_NONE;
}

void
DebugStream::Flush()
{
}
