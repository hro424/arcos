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
/// @file   Libraries/Persistent/SessionAllocator.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  May 2008
///

//$Id: PersistentPageAllocator.cpp 423 2008-09-30 08:47:19Z hro $

#define SYS_DEBUG

#include <Debug.h>
#include <Ipc.h>
#include <PersistentPageAllocator.h>
#include <Server.h>
#include <System.h>
#include <sys/Config.h>

// Header for a free region of pages
struct Header
{
    Header  *next;  // Next free region
    size_t  count;  // Number of pages in the region, or
                    // Total free pages (for _free_list)
};

static Header*          _base IS_PERSISTENT;
static Header*          _top IS_PERSISTENT;
static Header*          _end IS_PERSISTENT;
static Header           _free_list IS_PERSISTENT;

#define MUTEX_INIT
#define MUTEX_LOCK
#define MUTEX_UNLOCK

void
_shmalloc_init(addr_t base, size_t size)
{
    _base = (Header *)PAGE_ALIGN(base);
    _top = _base;
    _end = _base + size;
    _free_list.next = _top;
    _free_list.count = 0;

    MUTEX_INIT;
}

static stat_t
shmalloc_release(addr_t addr)
{
    L4_Msg_t msg;
    L4_Put(&msg, MSG_PAGER_RELEASE, 1, &addr, 0, 0);
    return Ipc::Call(L4_Pager(), &msg, &msg);
}

addr_t
shmalloc(size_t count)
{
    Header* cur;
    Header* prev;
    Header* top;

    MUTEX_LOCK;

#if 0
    DOUT("head: %p top: %p\n", &_free_list, _top);
    for (cur = _free_list.next; cur != _top; cur = cur->next) {
        DOUT("cur: %p next: %p\n", cur, cur->next);
    }
#endif // SYS_DEBUG

    prev = &_free_list;
    for (cur = _free_list.next; cur != _top; cur = cur->next) {
        if (count <= cur->count) {
            if (count == cur->count) {
                prev->next = cur->next;
            }
            else {
                prev->next = (Header *)((addr_t)cur + count * PAGE_SIZE);
                prev->next->next = cur->next;
                prev->next->count = cur->count - count;
            }
            goto exit;
        }
        prev = cur;
    }

    top = (Header*)((addr_t)_top + count * PAGE_SIZE);
    if (_end < top) {
        return 0;
    }

    _top = top;
    prev->next = _top;

exit:
    MUTEX_UNLOCK;

    return (addr_t)cur;
}

static void
merge(Header *ptr)
{
    if (ptr->next != _top &&
        ptr->next == (Header *)(((addr_t)ptr + ptr->count * PAGE_SIZE))) {

        ptr->count += ptr->next->count;
        ptr->next = ptr->next->next;
        merge(ptr);
    }
}

void
shmfree(addr_t page, size_t count)
{
    Header  *cur;
    Header  *prev;
    Header  *ptr = (Header *)(page & PAGE_MASK);

    if (ptr < _base || _top < (Header *)((addr_t)ptr + count * PAGE_SIZE)) {
        FATAL("pa:free");
        return;
    }

    // Release the pages because they might be shared with another task.
    // Assuming that the pages were allocated one by one.
    for (size_t i = 0; i < count; i++) {
        shmalloc_release(page + PAGE_SIZE * i);
    }

    //TODO: The following code generates a page fault on the page that has
    //      just release above.
    //
    //      When allocating a shared page, it must be released.  Remapping in
    //      the following code breaks this rule.

    ptr->next = 0;
    ptr->count = count;

    MUTEX_LOCK;

    prev = &_free_list;
    for (cur = _free_list.next; cur != _top; cur = cur->next) {
        if (ptr < cur) {
            break;
        }
        prev = cur;
    }
    ptr->next = cur;
    prev->next = ptr;
    merge(prev);

#if 0
    DOUT("head: %p top: %p\n", &_free_list, _top);
    for (cur = _free_list.next; cur != _top; cur = cur->next) {
        DOUT("cur: %p, cur->next: %p\n", cur, cur->next);
    }
#endif // SYS_DEBUG

    MUTEX_UNLOCK;
}

