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
/// @file   Libraries/Arc/file.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//#define SYS_DEBUG
//#define SYS_DEBUG_CALL

#include <Debug.h>
#include <FileStream.h>
#include <Ipc.h>
#include <Session.h>
#include <String.h>
#include <Types.h>

stat_t
FileStream::Open(const char *path, UInt mode)
{
    stat_t      err;
    L4_Word_t   reg[2];

    ENTER;

    // Send the path name through the shared memory
    memcpy(reinterpret_cast<void*>(_ss->GetBaseAddress()),
           path, strlen(path) + 1);

    reg[0] = mode;
    err = _ss->Begin(reg, 1, reg, 2);
    if (err != ERR_NONE) {
        return err;
    }
    _size = reg[1];

    _offset = 0;

    EXIT;
    return ERR_NONE;
}


stat_t
FileStream::Write(const void *buffer, size_t count, size_t* wsize)
{
    addr_t      ptr;
    UInt        offset;
    Int         length;
    Int         len;
    stat_t      err;

    if (buffer == 0) {
        return ERR_INVALID_ARGUMENTS;
    }

    if (count == 0) {
        if (wsize != 0) {
            *wsize = 0;
        }
        return ERR_NONE;
    }

    ptr = (addr_t)buffer;
    //offset = (UInt)SessionGetExtra(_ss);
    offset = _offset;
    length = (Int)count;

    while (0 < length) {
        Int ssize = static_cast<Int>(_ss->Size());
        len = (length < ssize) ? length : ssize;
        memcpy(reinterpret_cast<void*>(_ss->GetBaseAddress()),
               reinterpret_cast<const void*>(ptr), len);

        L4_Word_t reg[2];
        reg[0] = len;
        reg[1] = offset;
        
        err = _ss->Put(reg, 2, reg, 1);
        if (err != ERR_NONE) {
            return err;
        }

        Int wrote = reg[0];
        if (wrote == 0) {
            break;
        }

        ptr += wrote;
        offset += wrote;
        length -= wrote;
    }

    //SessionSetExtra(_ss, (void *)offset);
    _offset = offset;
    if (wsize != 0) {
        *wsize = length;
    }
    return ERR_NONE;
}

