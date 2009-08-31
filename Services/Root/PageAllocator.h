/*
 *
 *  Copyright (C) 2007, 2008, 2009, Waseda University.
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


#ifndef ARC_PAGE_ALLOC_H
#define ARC_PAGE_ALLOC_H

#include <Debug.h>
#include <String.h>
#include <Types.h>
#include "PageFrameTable.h"

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

    ///
    /// Sets the whole physical page to 0.
    ///
    void Clear(PageFrame *frame)
    {
        L4_Word_t   addr = _pft->GetAddress(frame);
        memset((void *)addr, 0, PAGE_SIZE);
    }

public:
    PageAllocator() : _pft(0) {}

    virtual ~PageAllocator() {}

    ///
    /// Initialization method
    /// @param The PageFrameTable describing the memory area to manage.
    ///
    void Initialize(PageFrameTable *pft)
    {
        ENTER;
        _pft = pft;
#ifdef BUDDY_ALLOCATOR
        // The memory after PFT is reserved for page allocate objects.
        // See CreateMainPft().
        //_ba = (BuddyAllocator *)(pft->Table() + pft->Length());
        _ba.Initialize(pft);
#endif // BUDDY_ALLOCATOR
        EXIT;
    }


    ///
    /// Allocate a memory block of count pages.
    /// The allocated block is returned in frame.
    ///
    virtual stat_t Allocate(L4_Word_t count, PageFrame **frame);

    virtual stat_t Allocate(L4_Word_t count, addr_t *phys);

    virtual stat_t Release(PageFrame *frame);

    virtual stat_t Release(addr_t phys)
    { return this->Release(_pft->GetFrame(phys)); }

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

    virtual void PrintMemoryUsage()
    {
#ifdef BUDDY_ALLOCATOR
        _ba.PrintMemoryUsage();
#endif
    }
};

#endif // ARC_PAGE_ALLOC_H

