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
/// @file   Services/Root/Pager.cc
/// @brief  The root pager
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

// $Id: Pager.cc 353 2008-05-30 04:40:04Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <MemoryPool.h>
#include <Protocol.h>
#include <String.h>
#include <Types.h>
#include <sys/Config.h>

#include "Common.h"
#include "PageAllocator.h"
#include "PageFrame.h"
#include "PageFrameTable.h"
#include "Pager.h"
#include "Space.h"
#include "Thread.h"

#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/types.h>

///
/// Check the end of the virtual address space of the user, which is the
/// beginning of the virtual address space for the kernel.
///
addr_t
Pager::ProbeKernelArea()
{
    L4_KernelInterfacePage_t*   kip =
        static_cast<L4_KernelInterfacePage_t*>(L4_GetKernelInterface());

    for (L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
        L4_MemoryDesc_t* desc = L4_MemoryDesc(kip, i);
        if (L4_IsMemoryDescVirtual(desc) &&
            L4_ConventionalMemoryType == (L4_MemoryDescType(desc) & 0xF)) {
            return L4_MemoryDescHigh(desc) + 1;
        }
    }
    return ~0UL;
}


void
Pager::ZeroPage(PageFrame *dest)
{
    ENTER;
    memset((void *)_pft->GetAddress(dest), 0, PAGE_SIZE);
    EXIT;
}

void
Pager::CopyPage(PageFrame *dest, PageFrame *src)
{
    ENTER;
    *dest = *src;
    memcpy((void *)_pft->GetAddress(dest),
           (const void *)_pft->GetAddress(src),
           PAGE_SIZE);
    DOUT("%.8lX -> %.8lX\n", _pft->GetAddress(src), _pft->GetAddress(dest));
    EXIT;
}

stat_t
Pager::MapSigma0(L4_Fpage_t fpage, L4_Fpage_t backing)
{
    L4_MapItem_t    mapItem;
    L4_Msg_t        msg;
    L4_Word_t       reg[2];
    stat_t          err;
    ENTER;

    // This function sets up the window to receive the mapping. The size of
    // window affects to 'send base' variable so that the parent pager of this
    // pager safely maps the fpage.
    L4_Accept(L4_MapGrantItems(fpage));	// Accept both Map and Grant operations

    reg[0] = backing.raw;
    reg[1] = 0;
    L4_Put(&msg, MSG_SIGMA0, 2, reg, 0, 0);

    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }
    //L4_Accept(L4_MapGrantItems(L4_Nilpage));

    L4_MsgGetMapItem(&msg, 0, &mapItem);

    EXIT;
    return ERR_NONE;
}

stat_t
Pager::CreateMap(addr_t address, L4_Word_t rwx, L4_Word_t sndbase,
                 L4_MapItem_t *map)
{
    L4_Fpage_t      fpage;
    stat_t          err;

    ENTER;
    DOUT("%.8lX -> %.8lX\n", address, sndbase);

    fpage = L4_FpageLog2(address, PAGE_BITS);
    L4_Set_Rights(&fpage, rwx);

    // Get the pager's page backed by S0's page (at the same address)
    err = MapSigma0(fpage, fpage);
    if (err != ERR_NONE) {
        System.Print(System.ERROR, "MapSigma0 failed\n");
        return err;
    }

    // Map the backed page to the requested address
    // NOTE: Assuming that the destination accepts the entire address space.
    // i.e. L4_Accept(L4_Fpage(L4_CompleteAddressSpace)).
    *map = L4_MapItem(fpage, sndbase & PAGE_MASK);
#ifdef SYS_DEBUG
    System.Print(System.INFO, "Map %.8lX sndbase: %.8lX\n",
                 map->raw[1], map->raw[0]);
#endif
    EXIT;
    return ERR_NONE;
}


Bool
Pager::IsValidAddress(addr_t address)
{
    return (address < ProbeKernelArea());
}

stat_t
Pager::CreateMapItem(addr_t dest, PageFrame *frame, L4_Word_t rwx,
                     L4_Word_t count, L4_MapItem_t *items)
{
    ENTER;
    /*
    for (L4_Word_t i = 0; i < count; i++) {
        if ((frame[i].Rights() & rwx) != rwx) {
            return ERR_INVALID_RIGHTS;
        }
    }
    */

    for (L4_Word_t i = 0; i < count; i++) {
        //FIXME: this condition is implicit to outside the function.
        //if (frame[i].Attribute & PAGE_ATTR_CONST == 0) {
        //if (frame[i].Mapping == 0UL) {
        if (frame[i].GetState() != PAGE_STATE_MAP) {
            frame[i].SetDestination(dest);
        }

        frame[i].SetState(PAGE_STATE_MAP);
        frame[i].AddAccessState(rwx);

        //FIXME: this is also implicit behavior!
        if (frame->IsCOW()) {
            CreateMap(_pft->GetAddress(&frame[i]), rwx, dest, &items[i]);
        }
        else {
            frame[i].SetOwnerRights(frame[i].GetOwnerRights() | rwx);
            CreateMap(_pft->GetAddress(&frame[i]), frame[i].GetOwnerRights(),
                      dest, &items[i]);
        }

        dest += PAGE_SIZE;
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Pager::CreateMapItem(addr_t dest, PageFrame *frame, L4_Word_t rwx,
                     L4_MapItem_t *item)
{
    return this->CreateMapItem(dest, frame, rwx, 1, item);
}

stat_t
Pager::Unmap(PageFrame *frame, L4_Word_t rwx)
{
    L4_Fpage_t  fpage;
    L4_Fpage_t  ret;

    frame->SetState(PAGE_STATE_UNMAP);
    fpage = L4_FpageLog2(_pft->GetAddress(frame), PAGE_BITS);
    L4_Set_Rights(&fpage, rwx);
    ret = L4_Flush(fpage);

    return ERR_NONE;
}

addr_t
Pager::PhysicalAddress(PageFrame *frame)
{
    return _pft->GetAddress(frame);
}

