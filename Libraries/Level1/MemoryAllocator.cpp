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
/// @file   Libraries/Level1/MemoryAllocator.cpp
/// @brief  A memory block allocator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: MemoryAllocator.cpp 374 2008-08-07 05:48:21Z hro $

#include <Assert.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include <Types.h>

struct Header
{
    size_t  count;
    union {
        Header  *next;
        word_t  checksum;
    };
};

static Header   *_base;         // Base address of the heap area
static Header   _free_list;     // List header for free blocks

#define MUTEX_INIT
#define MUTEX_LOCK
#define MUTEX_UNLOCK


static word_t
mkchecksum(Header *h)
{
    return 0xDEADBEEF;
}

static Bool
check(Header *h)
{
    return (h->checksum == 0xDEADBEEF);
}

static void
merge(Header *ptr)
{
    if (ptr->next != 0 &&
        ptr->next == (Header *)((addr_t)ptr + ptr->count)) {

        ptr->count += ptr->next->count;
        ptr->next = ptr->next->next;
        merge(ptr);
    }
}

static void
release(void *p, size_t count)
{
    Header  *cur;
    Header  *prev;
    Header  *ptr = (Header *)p;

    assert(_base <= ptr);

    ptr->next = 0;
    ptr->count = count;

    MUTEX_LOCK;

    prev = &_free_list;
    for (cur = _free_list.next; cur != 0; cur = cur->next) {
        assert(_base <= cur);
        if (ptr < cur) {
            break;
        }
        prev = cur;
    }

    assert(ptr != 0);
    ptr->next = cur;

    assert(prev != 0);
    prev->next = ptr;
    merge(prev);

    MUTEX_UNLOCK;
}

static Bool
expand(void)
{
    Header  *page;

    page = (Header *)palloc(1);
    if (page == 0) {
        return FALSE;
    }

    release(page, PAGE_SIZE);

    return TRUE;
}

static void *
allocate(size_t count, word_t align)
{
    Header  *cur;
    Header  *prev;

    // Find a free block that fits or is larger than the request.
retry:
    MUTEX_LOCK;

    prev = &_free_list;
    for (cur = _free_list.next; cur != 0; cur = cur->next) {
        if (count == cur->count) {
            prev->next = cur->next;
            goto exit;
        }
        else if (count + sizeof(Header) <= cur->count) {
            prev->next = (Header *)((addr_t)cur + count);
            prev->next->next = cur->next;
            prev->next->count = cur->count - count;
            goto exit;
        }
        prev = cur;
    }

    MUTEX_UNLOCK;

    if (!expand()) {
        return (void *)0;
    }

    goto retry;

exit:
    MUTEX_UNLOCK;

    assert(_base <= cur);
    return (void *)cur;
}

void
_malloc_init()
{
    _base = (Header *)palloc(1);
    if (_base == 0) {
        FATAL("ma:init");
    }

    _free_list.next = 0;
    _free_list.count = 0;

    MUTEX_INIT;

    release(_base, PAGE_SIZE);
}

//TODO: Deal with align!
void *
_malloc_align(size_t count, size_t align)
{
    Header  *ptr;

    if (count < align) {
        count = align;
    }

    ptr = (Header *)allocate(count + sizeof(Header), align);
    if (ptr == 0) {
        return 0;
    }

    ptr->count = count + sizeof(Header);
    ptr->checksum = mkchecksum(ptr);

    return (void *)((addr_t)ptr + sizeof(Header));
}

void
_mfree(void *ptr)
{
    Header  *header;

    header = (Header *)((addr_t)ptr - sizeof(Header));
    if (!check(header)) {
        FATAL("ma:checksum\n");
        return;
    }

    release(header, header->count);
}

