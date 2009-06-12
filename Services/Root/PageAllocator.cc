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
/// @brief  Page alloctor base class
/// @file   Services/Root/PageAllocator.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: PageAllocator.cc 349 2008-05-29 01:54:02Z hro $

#include <Assert.h>
#include <String.h>
#include <Types.h>
#include "PageAllocator.h"
#include "PageFrameTable.h"

#ifdef BUDDY_ALLOCATOR
#include "BuddyAllocator.h"
#endif // BUDDY_ALLOCATOR

///
/// Sets the whole physical page to 0.
///
void
PageAllocator::Clear(PageFrame *frame)
{
    L4_Word_t   addr = _pft->GetAddress(frame);
    memset((void *)addr, 0, PAGE_SIZE);
}

void
PageAllocator::Initialize(PageFrameTable *pft)
{
    ENTER;
    _pft = pft;
#ifdef BUDDY_ALLOCATOR
    // The memory after PFT is reserved for page allocate objects.
    // See CreateMainPft().
    //_ba = (BuddyAllocator *)(pft->Table() + pft->Length());
    //DOUT("BuddyAllocator allocated @ %p\n", _ba);
    _ba.Initialize(pft);
#endif // BUDDY_ALLOCATOR
    EXIT;
}

stat_t
PageAllocator::Allocate(L4_Word_t count, PageFrame **frame)
{
    PageFrame   *obj;
    ENTER;

#ifdef BUDDY_ALLOCATOR
    L4_Word_t   exp;

    for (exp = 0; (1UL << exp) < count; exp++) ;

    if (_ba.Allocate(exp, &obj) != ERR_NONE) {
        return ERR_OUT_OF_MEMORY;
    }

    obj->SetPageGroup(1 << exp);
#endif // BUDDY_ALLOCATOR

    for (L4_Word_t i = 0; i < obj->GetPageGroup(); i++) {
        obj[i].Initialize();
        obj[i].RefCnt++;
        obj[i].SetState(PAGE_STATE_ALLOC);
        obj[i].SetOwner(L4_Myself());
    }
    *frame = obj;

    EXIT;
    return ERR_NONE;
}

stat_t
PageAllocator::Allocate(L4_Word_t count, addr_t *phys)
{
    PageFrame   *frame;
    stat_t      err;

    err = this->Allocate(count, &frame);
    if (err != ERR_NONE) {
        return err;
    }

    *phys = _pft->GetAddress(frame);
    return ERR_NONE;
}

stat_t
PageAllocator::Release(PageFrame *frame)
{
    ENTER;
    for (PageFrame *ptr = frame; ptr < frame + frame->GetPageGroup(); ptr++) {
        assert(_pft->IsValidFrame(ptr));
        ptr->RefCnt--;
        ptr->SetOwner(L4_nilthread);
        ptr->SetState(PAGE_STATE_FREE);
    }

#ifdef BUDDY_ALLOCATOR
    _ba.Release(frame);
#endif // BUDDY_ALLOCATOR
    EXIT;
    return ERR_NONE;
}

stat_t
PageAllocator::Release(addr_t phys)
{
    return this->Release(_pft->GetFrame(phys));
}

stat_t
PageAllocator::GetFrameForAddress(addr_t phys, PageFrame **frame)
{
    PageFrame *f = _pft->GetFrame(phys);
    if (!f) return ERR_NOT_FOUND;
    *frame = f;
    return ERR_NONE;
}

void
PageAllocator::PrintMemoryUsage()
{
#ifdef BUDDY_ALLOCATOR
    _ba.PrintMemoryUsage();
#endif
}

