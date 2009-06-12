/*
 *
 *  Copyright (C) 2006, 2007, 2008, Waseda University.
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
/// @file   Service/Root/Common.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.ac.jp>
/// @since  2006
///

// $Id: Common.h 383 2008-08-28 06:49:02Z hro $
#ifndef ARC_ROOT_RUNTIME_H
#define ARC_ROOT_RUNTIME_H

#include <Ipc.h>
#include <MemoryPool.h>
#include <l4/message.h>
#include <l4/types.h>

class MemoryPool;
class PageFrameTable;
class PageAllocator;

extern L4_ThreadId_t    RootId;
extern PageFrameTable   MainPft;
extern PageAllocator    MainPa;
extern MemoryPool       MemPool;


/// Heap size, in bytes
#define HEAP_SIZE       0x00100000UL


#ifdef SYS_DEBUG_ALLOC
#define MEM_TRACE   \
    ConsoleOut(DEBUG, "%20s %d: Heap usage: free:%u used:%u\n",   \
               __func__, __LINE__, MemPool.Unused(),            \
               MemPool.Length() - MemPool.Unused());
#else
#define MEM_TRACE
#endif


#define ROUND_UP(v, s)     (((v) / (s) + 1) * (s))

INLINE void*
malloc(size_t s)
{
    return MemPool.Allocate(s);
}

INLINE void
mfree(void* p)
{
    MemPool.Release(p);
}

INLINE void
NotifyReady(void)
{
    L4_Msg_t        msg;
    L4_MsgPut(&msg, 0, 0, (L4_Word_t *)0, 0, (void *)0);
    Ipc::Send(RootId, &msg);
}

INLINE void
WaitReady(L4_ThreadId_t tid)
{
    L4_Receive(tid);
}

INLINE L4_Word_t
min(L4_Word_t a, L4_Word_t b) {
    return (a < b) ? a : b;
}

INLINE L4_Word_t
max(L4_Word_t a, L4_Word_t b) {
    return (a > b) ? a : b;
}

#endif /* ARC_ROOT_RUNTIME_H */

