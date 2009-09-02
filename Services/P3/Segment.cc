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
/// @file   Services/P3/Segment.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///


#include <Ipc.h>
#include <MemoryManager.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>

#include "Pel.h"
#include "Segment.h"

stat_t
Segment_Head::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                       L4_Word_t *rwx)
{
#ifdef CPR
    // Make a snapshot
    if (faddr == 0x1FFFF000) {
        L4_Word_t       control;
        L4_Word_t       sp;
        L4_Word_t       dummy;
        L4_ThreadId_t   dummy_tid;

        // Obtain SP
        control = (1 << 9) | (1 << 4);
        L4_ExchangeRegisters(tid, control, 0, fip + 10, 0, 0, L4_nilthread,
                             &dummy, &sp, &dummy, &dummy, &dummy,
                             &dummy_tid);
        System.Print(INFO, "Snapshot @ %.8lX: ip %.8lX, sp %.8lX\n",
                   tid.raw, fip + 10, sp);

        L4_Msg_t msg;
        L4_Word_t reg[2];

        reg[0] = fip + 10;
        reg[1] = sp;
        L4_Put(&msg, MSG_ROOT_SNAPSHOT, 2, reg, 0, 0);
        return IpcCall(Pel::RootTask(), &msg, &msg);
    }
#endif // CPR

    System.Print(System.ERROR, "[%.8lX] Access violation.\n", L4_Myself().raw);
    System.Print(System.ERROR, "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
         faddr, fip, *rwx, tid.raw);
    *rwx = L4_NoAccess;
    return ERR_INVALID_RIGHTS;
}


stat_t
TextSegment::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                      L4_Word_t *rwx)
{
    if (!Hit(faddr)) {
        *rwx = L4_NoAccess;
        return ERR_OUT_OF_RANGE;
    }

    DOUT("text segment\n");

    // Check access rights
    /*
    if ((rwx & L4_Writable) != 0) {
        // Illegal access
        System.Print(ERROR, "Write access to the text section\n");
        return ERR_INVALID_RIGHTS;
    }
    */

    // Change the permission to the page to readable and executable in
    // case of either a read access or an execution access only, in order
    // to avoid futher page faults to the page.
    *rwx = L4_ReadeXecOnly;

    // Text section is already loaded by Pel, so it doesn't obtains a page
    // from the root task.

    return ERR_NONE;
}


stat_t
DataSegment::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                      L4_Word_t *rwx)
{
    if (!Hit(faddr)) {
        *rwx = L4_NoAccess;
        return ERR_OUT_OF_RANGE;
    }

    DOUT("data segment\n");

    /*
    if ((*rwx & L4_eXecutable) != 0) {
        System.Print(ERROR, "Execution access to the data section\n");
        System.Print(ERROR, "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                   faddr, fip, *rwx, tid.raw);
        *rwx = L4_NoAccess;
        return ERR_INVALID_RIGHTS;
    }
    */

    *rwx = L4_ReadWriteOnly;

    // Data section is already loaded by P3, so it doesn't obtains a page
    // from the root task.
    // XXX: However, explicit mapping is necessary if you enable snapshotting.
    Pager.Map(faddr, *rwx, 0, L4_Myself());

    return ERR_NONE;
}

void
PersistentSegment::Initialize(addr_t start, addr_t end)
{
    _start = start;
    _end = end;

    for (addr_t addr = _start; addr < _end; addr += PAGE_SIZE) {
        Pager.Map(addr, L4_ReadWriteOnly);
    }

    DOUT("PM segment: %.8lX - %.8lX\n", _start, _end);
}

stat_t
PersistentSegment::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                            L4_Word_t *rwx)
{
    ENTER;
    if (!Hit(faddr)) {
        *rwx = L4_NoAccess;
        return ERR_OUT_OF_RANGE;
    }

    DOUT("persistent segment\n");

    /*
    if ((*rwx & L4_eXecutable) != 0) {
        System.Print(ERROR, "Execution access to the pm section\n");
        System.Print(ERROR, "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                   faddr, fip, *rwx, tid.raw);
        *rwx = L4_NoAccess;
        return ERR_INVALID_RIGHTS;
    }
    */

    *rwx = L4_ReadWriteOnly;
    // Request one page to the root task
    //Map(faddr, 0, L4_anythread, 1, *rwx);
    EXIT;
    return ERR_NONE;
}

stat_t
HeapSegment::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                      L4_Word_t *rwx)
{
    if (!Hit(faddr)) {
        *rwx = L4_NoAccess;
        return ERR_OUT_OF_RANGE;
    }

    DOUT("heap segment\n");
    /*
    if ((*rwx & L4_eXecutable) != 0) {
        System.Print(ERROR, "Execution access to the heap\n");
        System.Print(ERROR, "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                   faddr, fip, *rwx, tid.raw);
        *rwx = L4_NoAccess;
        return ERR_INVALID_RIGHTS;
    }
    *rwx = L4_ReadWriteOnly;
    */
    *rwx = L4_FullyAccessible;
    // Request one page to the root task
    Pager.Map(faddr, *rwx);
    return ERR_NONE;
}

stat_t
StackSegment::HandlePf(L4_ThreadId_t tid, L4_Word_t faddr, L4_Word_t fip,
                       L4_Word_t *rwx)
{
    if (!Hit(faddr)) {
        *rwx = L4_NoAccess;
        return ERR_OUT_OF_RANGE;
    }

    DOUT("stack segment\n");

    /*
    if ((*rwx & L4_eXecutable) != 0) {
        System.Print(ERROR, "[%.8lX] Execution access to non-text area.\n",
                   L4_Myself().raw);
        System.Print(ERROR, "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                   faddr, fip, *rwx, tid.raw);
        *rwx = L4_NoAccess;
        return ERR_INVALID_RIGHTS;
    }
    */

    // The first page of the stack is already mapped.
    if (_cursor == _end) {
        _cursor = _start;
        _start -= PAGE_SIZE;
        return ERR_NONE;
    }

    *rwx = L4_ReadWriteOnly;
    Pager.Map(faddr, *rwx);
    Grow();

    return ERR_NONE;
}

/*
addr_t
StackSegment::SetupArgs(L4_Word_t *args, size_t count)
{
    L4_Word_t   *ptr;
    ENTER;

    ptr = reinterpret_cast<L4_Word_t *>(_cursor - sizeof(L4_Word_t) * count);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = args[i];
    }

    //_cursor -= PAGE_SIZE;

    EXIT;
    return (addr_t)ptr;
}
*/

addr_t
StackSegment::SetupArgs(addr_t heap, Int argc, char* argv[], Int state)
{
    L4_Word_t*  stack;
    size_t      len;
    char*       dest;
    ENTER;

    // Skip the last one word (@0xbffffc) that is never mapped
    len = sizeof(L4_Word_t) * (8 + argc);
    for (Int i = 0; i < argc; i++) {
        DOUT("argv[%d] '%s'\n", i, argv[i]);
        len += strlen(argv[i]) + 1;
    }
    len = (len / sizeof(L4_Word_t) + 1) * sizeof(L4_Word_t);

    stack = reinterpret_cast<L4_Word_t*>(_cursor - len);
    stack[0] = static_cast<L4_Word_t>(heap);
    stack[1] = static_cast<L4_Word_t>(argc);
    stack[2] = reinterpret_cast<L4_Word_t>(&stack[4]);
    stack[3] = static_cast<L4_Word_t>(state);

    dest = reinterpret_cast<char*>(&stack[4 + argc]);

    for (Int i = 0; i < argc; i++) {
        size_t alen = strlen(argv[i]) + 1;
        memcpy(dest, argv[i], alen);
        stack[4 + i] = reinterpret_cast<L4_Word_t>(dest);

        dest += alen;
    }

    EXIT;
    return reinterpret_cast<addr_t>(stack);
}


