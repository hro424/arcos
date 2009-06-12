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
/// @file   Services/Root/BuddyAllocator.cc
/// @brief  Page allocator based on the buddy algorithm
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

// $Id: BuddyAllocator.cc 349 2008-05-29 01:54:02Z hro $

#include <Assert.h>
#include <Debug.h>
#include <MemoryPool.h>
#include <Mutex.h>
#include <String.h>
#include <System.h>
#include <Types.h>
#include "Common.h"
#include "PageAllocator.h"
#include "PageFrame.h"
#include "PageFrameTable.h"

#include <l4/kip.h>
#include <l4/types.h>

static inline unsigned int
dec2exp(unsigned int d)
{
    unsigned int e;
    for (e = 0; (1U << e) < d; e++) ;
    return e;
}

static inline unsigned int
exp2dec(unsigned int e)
{
    return 1 << e;
}

unsigned int
BuddyAllocator::FindBuddy(unsigned int idx, unsigned int order)
{
    return idx ^ (1 << order);
}

unsigned int
BuddyAllocator::FindParentIndex(unsigned int idx, unsigned int order) {
    return (idx & ~(1 << order));
}

stat_t
BuddyAllocator::Split(PageFrame *frame, PageFrame **buddy)
{
    unsigned int    order = dec2exp(frame->GetPageGroup());

    if (MAX_ORDER <= order) {
        return ERR_OUT_OF_RANGE;
    }

    for (PageFrame *cur = frame; cur < frame + (1 << order); cur++) {
        cur->SetPageGroup(cur->GetPageGroup() >> 1);
    }

    *buddy = frame + frame->GetPageGroup();

    return ERR_NONE;
}

stat_t
BuddyAllocator::Merge(PageFrame **frame, PageFrame *buddy)
{
    PageFrame       *ptr = *frame;
    unsigned int    order = dec2exp(ptr->GetPageGroup());

    if (MAX_ORDER - 1 <= order) {
        return ERR_OUT_OF_RANGE;
    }

    // Update the pointer to the lowest address
    if (buddy < ptr) {
        *frame = buddy;
    }

    // Increase all the concerned physical pages to reflect the
    // new size.
    for (PageFrame *cur = *frame; cur < *frame + (1 << (order + 1)); cur++) {
        cur->SetPageGroup(cur->GetPageGroup() << 1);
    }

    return ERR_NONE;
}

void
BuddyAllocator::Push(unsigned int order, PageFrame *frame) {
    frame->next = _bins[order].link;
    _bins[order].link = frame;
    _bins[order].count++;
}

PageFrame *
BuddyAllocator::Pop(unsigned int order)
{
    PageFrame *f = _bins[order].link;
    _bins[order].link = f->next;
    _bins[order].count--;
    f->next = 0;
    return f;
}

void
BuddyAllocator::Remove(unsigned int order, PageFrame *f)
{
    PageFrame **pp = &_bins[order].link;
    while (*pp != f) {
        pp = &((*pp)->next);
    }
    *pp = f->next;
    f->next = 0;
    _bins[order].count--;
}

bool
BuddyAllocator::IsEmpty(PageBin *bin)
{
    //return (bin->link.next == &bin->link);
    return (bin->link == 0);
}

stat_t
BuddyAllocator::AcquireBlock(unsigned int order, PageFrame **frame)
{
    if (order >= MAX_ORDER) {
        return ERR_OUT_OF_MEMORY;
    }

    // No entries in the free list?
    if (IsEmpty(&_bins[order])) {
        PageFrame   *block;
        PageFrame   *buddy;

	// Then we have to request a block of the next order of magnitude
	// and split it
        if (AcquireBlock(order + 1, &block) != ERR_NONE) {
            return ERR_OUT_OF_MEMORY;
        }

        if (Split(block, &buddy) != ERR_NONE) {
            return ERR_UNKNOWN;
        }

        _bins[order].mutex.Lock();
        Push(order, buddy);
        _bins[order].mutex.Unlock();
        *frame = block;
    }
    // We have a free block that corresponds
    else {
        *frame = Pop(order);
        _bins[order].mutex.Unlock();
    }

    return ERR_NONE;
}

///
/// Releases the block given as argument. If its buddy is also free,
/// the two blocks are joined.
///
stat_t
BuddyAllocator::ReleaseBlock(PageFrame *frame)
{
    PageFrame   *buddy;
    L4_Word_t   order = dec2exp(frame->GetPageGroup());

    assert(_pft->IsValidFrame(frame));

    if (MAX_ORDER <= order) {
        return ERR_OUT_OF_RANGE;
    }

    _bins[order].mutex.Lock();

    // We will check if it is possible to merge the released block with
    // its buddy - only makes sense if there is a superior order of
    // magnitude to merge into.
    if (order < MAX_ORDER - 1) {
        L4_Word_t       index;
       
        index = FindBuddy(_pft->GetFrameIndex(frame), order);
        buddy = frame - _pft->GetFrameIndex(frame) + index;

        assert(_pft->IsValidFrame(buddy));

	// If the buddy is free and not divided, we can merge the two blocks
        if (buddy->GetPageGroup() == frame->GetPageGroup() &&
            buddy->GetType() == PAGE_TYPE_CONVENTIONAL &&
            buddy->GetState() == PAGE_STATE_FREE) {

            Remove(order, buddy);
            _bins[order].mutex.Unlock();

            if (Merge(&frame, buddy) != ERR_NONE) {
                FATAL("failed to merge buddies");
            }
	    // Continue to try merging until no more possible
            return ReleaseBlock(frame);
        }
    }

    // Add the released (and maybe resized) block to our bin
    Push(order, frame);
    _bins[order].mutex.Unlock();

    return ERR_NONE;
}

stat_t
BuddyAllocator::Allocate(unsigned int order, PageFrame **page)
{
    //unsigned int    order;
    PageFrame       *frame;
    ENTER;

    assert(order < MAX_ORDER);

    // Get a chunk of frames
    if (AcquireBlock(order, &frame) != ERR_NONE) {
        return ERR_OUT_OF_MEMORY;
    }

    assert(frame != 0);

    *page = frame;
    _allocated += (1UL << order);

    EXIT;
    return ERR_NONE;
}

stat_t
BuddyAllocator::Release(PageFrame *frame)
{
    stat_t    err;
    ENTER;

    _allocated -= frame->GetPageGroup();
    err = ReleaseBlock(frame);

    EXIT;
    return err;
}

void
BuddyAllocator::PrintMemoryUsage()
{
/*
    Int free_count = 0;

    printf("-- Buddy Allocator Dump --\n");
    printf("order count   pages\n");
    for (UInt i = 0; i < MAX_ORDER; i++) {
        free_count += (1 << i) * _bins[i].count;
        printf("%5lu %5u %7u\n", i, _bins[i].count, (1 << i) * _bins[i].count);
    }

    printf("%8ld pages are in use\n", _allocated);
    printf("%8ld pages are free\n", free_count);
    printf("%8ld pages are reserved\n", _reserved);
    printf("-- Buddy Allocator Dump End --\n");
*/
}

void
BuddyAllocator::Initialize(PageFrameTable *pft)
{
    PageFrame   *head = pft->Table();

    ENTER;

    assert(pft != 0);
    _pft = pft;

    // Reset all bins...
    for (unsigned int i = 0; i < MAX_ORDER; i++) {
        memset(&_bins[i], 0, sizeof(PageBin));
        _bins[i].link = 0;
        _bins[i].count = 0;
        _bins[i].mutex.Initialize();
    }

    _allocated = 0;
    _reserved = 0;

    // All pages of physical memory should be set as free,
    // and have a length of 1.
    for (unsigned int i = 0; i < pft->Length(); i++) {
        if (head[i].GetType() == PAGE_TYPE_CONVENTIONAL &&
            head[i].GetState() == PAGE_STATE_ALLOC) {

            head[i].SetState(PAGE_STATE_FREE);
            if (head[i].GetPageGroup() != 1) {
                FATAL("Err_BuddyAllocatorInitialize");
            }

	    // Releasing all blocks will correctly initialize the different
	    // bins to cover all reachable memory.
            if (ReleaseBlock(&head[i]) != ERR_NONE) {
                FATAL("Err_BuddyAllocatorInitialize");
            }
        }
        else {
            _reserved++;
        }
    }

    EXIT;
}

