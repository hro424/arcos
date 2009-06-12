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
/// @brief  Memory manager API
/// @file   Include/arc/MemoryManager.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: MemoryManager.cpp 349 2008-05-29 01:54:02Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <Types.h>
#include <sys/Config.h>
#include <l4/space.h>
#include <l4/types.h>
#include <l4/message.h>

MemoryManager   Pager;

stat_t
MemoryManager::Reserve(addr_t base, L4_ThreadId_t peer, UInt perm)
{
    L4_Msg_t    msg;
    L4_Word_t   reg[3];

    reg[0] = base;
    reg[1] = 1;
    reg[2] = L4_ThreadNo(peer) << 14 | perm;

    L4_Put(&msg, MSG_PAGER_ALLOCATE | L4_ReadWriteOnly, 3, reg, 0, 0);
    return Ipc::Call(L4_Pager(), &msg, &msg);
}

stat_t
MemoryManager::Reserve(addr_t base)
{
    return Reserve(base, L4_Myself(), 0);
}

stat_t
MemoryManager::Release(addr_t base)
{
    L4_Msg_t msg;
    L4_Put(&msg, MSG_PAGER_RELEASE, 1, &base, 0, 0);
    return Ipc::Call(L4_Pager(), &msg, &msg);
}

stat_t
MemoryManager::Expand(size_t count)
{
    L4_Msg_t msg;
    L4_Put(&msg, MSG_PAGER_EXPAND, 1, (L4_Word_t *)&count, 0, 0);
    return Ipc::Call(L4_Pager(), &msg, &msg);
}

Bool
MemoryManager::IsMapped(addr_t address, UInt rwx)
{
    L4_Fpage_t fpage;

    fpage = L4_Fpage(address, PAGE_SIZE);
    fpage = L4_GetStatus(fpage);

    return (fpage.raw & rwx) == rwx;
}

stat_t
MemoryManager::Map(addr_t dest, L4_ThreadId_t did, UInt rwx,
                   addr_t src, L4_ThreadId_t sid)
{
    L4_Msg_t    msg;
    L4_Word_t   reg[5];
    stat_t      err;

    if (IsMapped(dest, rwx)) {
        return ERR_NONE;
    }

    reg[0] = dest;
    reg[1] = did.raw;
    reg[2] = src;
    reg[3] = sid.raw;
    reg[4] = 1;

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));
    L4_Put(&msg, MSG_PAGER_MAP | (rwx & 0x7), 5, reg, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    if (err != ERR_NONE) {
        return err;
    }

    return ERR_NONE;
}

stat_t
MemoryManager::Map(addr_t dest, UInt rwx, addr_t src, L4_ThreadId_t sid)
{
    return Map(dest, L4_Myself(), rwx, src, sid);
}

stat_t
MemoryManager::Map(addr_t dest, UInt rwx)
{
    return Map(dest, L4_Myself(), rwx, 0, L4_anythread);
}

addr_t
MemoryManager::Phys(addr_t virt)
{
    L4_Msg_t    msg;
    addr_t      offset;

    offset = virt & ~PAGE_MASK;
    L4_Put(&msg, MSG_PAGER_PHYS, 1, &virt, 0, 0);
    if (Ipc::Call(L4_Pager(), &msg, &msg) != ERR_NONE) {
        return ~0UL;
    }
    return static_cast<addr_t>(L4_Get(&msg, 0)) + offset;
}

