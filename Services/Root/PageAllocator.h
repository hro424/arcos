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
/// @file   Services/Root/PageAllocator.h
/// @brief  Physical page allocator interface
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: PageAllocator.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_PAGE_ALLOC_H
#define ARC_PAGE_ALLOC_H

#include <Types.h>
#ifdef BUDDY_ALLOCATOR
#include "BuddyAllocator.h"
#endif // BUDDY_ALLOCATOR

class PageFrame;
class PageFrameTable;

class PageAllocator
{
private:
    PageFrameTable  *_pft;
#ifdef BUDDY_ALLOCATOR
    BuddyAllocator  _ba;
#endif // BUDDY_ALLOCATOR

    void Clear(PageFrame *frame);

public:
    PageAllocator() : _pft(0) {}

    virtual ~PageAllocator() {}

    ///
    /// Initialization method
    /// @param The PageFrameTable describing the memory area to manage.
    ///
    void Initialize(PageFrameTable *pft);

    ///
    /// Allocate a memory block of count pages.
    /// The allocated block is returned in frame.
    ///
    virtual stat_t Allocate(L4_Word_t count, PageFrame **frame);
    virtual stat_t Allocate(L4_Word_t count, addr_t *phys);

    virtual stat_t Release(PageFrame *frame);
    virtual stat_t Release(addr_t phys);

    /// 
    /// Get the PageFrame corresponding to the physical address given in
    /// parameter.
    /// 
    stat_t GetFrameForAddress(addr_t phys, PageFrame **frame);

    ///
    /// Obtains the total count of pages managed by the allocator.
    ///
    //virtual size_t Length();

    ///
    /// Obtains the number of pages in use.
    ///
    //virtual size_t Used();

    virtual void PrintMemoryUsage();
};

#endif // ARC_PAGE_ALLOC_H

