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
/// @file   Services/Devices/Sata/MemoryPool.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: MemoryPool.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/ipc.h>
#include <arc/memory.h>
#include <arc/protocol.h>
#include <arc/system.h>
#include <arc/status.h>
#include "MemoryPool.h"

#include <l4/ipc.h>

status_t
MemoryPool::Initialize(size_t size)
{
    /*
    status_t        err;
    L4_Msg_t        msg;
    L4_MapItem_t    item;
    L4_Word_t       count;
    L4_Word_t       reg[2];
    */

    ENTER;
    DOUT("size: %u bytes\n", size);

    // Get anonymous pages
    //Map(_base, L4_Myself(), 0, L4_anythread, count, L4_ReadWriteOnly);
    _base = mmap(0, 0, L4_anythread, size, L4_ReadWriteOnly);

    _cursor = _base;
    _size = PAGE_ALIGN(size);
    _end = _base + _size;

    /*
    reg[0] = PAGE_MASK;
    reg[1] = L4_Myself().raw;
    reg[2] = count;

    DOUT("Map memory pool\n");

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));
    L4_Put(&msg, MSG_PEL_MAP | L4_ReadWriteOnly, 2, reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        printf(ERROR, "Ipc: %u\n", err);
        Arc_Break();
        return err;
    }

    _basePhys = L4_Get(&msg, 0);
    L4_Get(&msg, 0, &item);
    _baseVirt = L4_Address(L4_SndFpage(item));
    DOUT("virt: 0x%.8X, phys: 0x%.8X\n", _baseVirt, _basePhys);
    _cursor = _baseVirt;
    _size = PAGE_ALIGN(size);
    _endVirt = _baseVirt + _size;
    */

    EXIT;
    return ERR_NONE;
}

status_t
MemoryPool::AllocateAlign(size_t size, addr_t align, void **obj)
{
    UInt    ptr;

    DOUT("cursor: 0x%.8lX, size: %u, align: %lu\n", _cursor, size, align);
    ptr = ((_cursor + align - 1) / align) * align;
    if (ptr + size >= _end) {
        return ERR_OUT_OF_MEMORY;
    }

    _cursor = ptr + size;
    *obj = (void *)ptr;

    return ERR_NONE;
}

unsigned long
MemoryPool::Phys(unsigned long virt)
{
    return getphys(virt);
    //return virt - _virt + _phys;
}

