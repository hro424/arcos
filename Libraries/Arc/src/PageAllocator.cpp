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
/// @file   Libraries/Arc/PageAllocator.cc
/// @brief  A simple page allocator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: PageAllocator.cpp 349 2008-05-29 01:54:02Z hro $

#include <Ipc.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include <sys/Config.h>

extern void _palloc_init(addr_t base);
extern addr_t _palloc(size_t count);
extern void _pfree(addr_t page, size_t count);

addr_t
palloc(size_t count)
{
    return _palloc(count);
}

addr_t
palloc_shm(size_t count, L4_ThreadId_t tid, UInt perm)
{
    addr_t      base;

    base = palloc(count);
    if (base != 0) {
        addr_t tmp = base;
        for (size_t i = 0; i < count; i++) {
            //FIXME: atomic {
            Pager.Release(tmp);
            if (Pager.Reserve(tmp, tid, perm) != ERR_NONE) {
                //FIXME: } atomic
                for (size_t j = 0; j < i; j++) {
                    Pager.Release(base + j * PAGE_SIZE);
                }
                pfree(base, count);
                return 0;
            }
            tmp += PAGE_SIZE;
        }
    }

    return base;
}

void
pfree(addr_t page, size_t count)
{
    _pfree(page, count);
}

