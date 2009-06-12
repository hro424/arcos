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
/// @file   Services/Root/PageFrameTable.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: PageFrameTable.cc 353 2008-05-30 04:40:04Z hro $

#include <Debug.h>
#include <MemoryPool.h>
#include <System.h>
#include <String.h>
#include <sys/Config.h>
#include <l4/kip.h>

#include "BuddyAllocator.h"
#include "Common.h"
#include "PageAllocator.h"
#include "PageFrame.h"
#include "PageFrameTable.h"


static void
PrintEntry(L4_Word_t low, L4_Word_t high, L4_Word_t type, L4_Word_t state,
           L4_Word_t attr, L4_Word_t rwx)
{
    System.Print(" %.8lX -  %.8lX ", low, high);
    switch (type) {
        case L4_UndefinedMemoryType:
            System.Print("Undefined    ");
            break;
        case L4_ConventionalMemoryType:
            System.Print("Conventional ");
            break;
        case L4_ReservedMemoryType:
            System.Print("Reserved     ");
            break;
        case L4_DedicatedMemoryType:
            System.Print("Dedicated    ");
            break;
        case L4_SharedMemoryType:
            System.Print("Shared       ");
            break;
#ifdef NICTA_EMBEDDED
        case L4_TracebufferMemoryType:
            System.Print("Trace buffer ");
            break;
#endif
        case L4_BootLoaderSpecificMemoryType:
            System.Print("Bootloader   ");
            break;
        case L4_ArchitectureSpecificMemoryType:
            System.Print("Architecture ");
            break;
        default:
            System.Print("Unknown      ");
    }

    switch (state) {
        case PAGE_STATE_FREE:
            System.Print("Free       ");
            break;
        case PAGE_STATE_ALLOC:
            System.Print("Allocated  ");
            break;
        case PAGE_STATE_MAP:
            System.Print("Mapped     ");
            break;
        case PAGE_STATE_UNMAP:
            System.Print("Unmapped   ");
            break;
        default:
            System.Print("Unknown    ");
            break;
    }

    switch (attr) {
        case PAGE_ATTR_COW:
            System.Print("COW       ");
            break;
        default:
            System.Print("%lu          ", attr);
            break;
    }
 
    if (rwx & PAGE_PERM_READ) {
        System.Print("r");
    }
    else {
        System.Print("-");
    }

    if (rwx & PAGE_PERM_WRITE) {
        System.Print("w");
    }
    else {
        System.Print("-");
    }

    if (rwx & PAGE_PERM_EXEC) {
        System.Print("x");
    }
    else {
        System.Print("-");
    }
    System.Print("\n");
}

void
PageFrameTable::Dump(void)
{
    L4_Word_t   type, state, attr, rwx, low, high;
    PageFrame   *cur = 0;

    low = _base;
    type = _table->GetType();
    state = _table->GetState();
    attr = _table->GetAttribute();
    rwx = _table->GetOwnerRights();

    System.Print("COUNT:0x%lX\n", _count);
    System.Print("  Begin        End        Type       State   Attributes Rights\n");
    System.Print("---------   --------- ------------ --------- ---------- ------\n");
    for (L4_Word_t i = 0; i < _count; i++) {
        cur = &_table[i];
        if (cur->GetType() != type ||
            cur->GetState() != state ||
            cur->GetAttribute() != attr ||
            cur->GetOwnerRights() != rwx) {

            high = GetAddress(cur) - 1;

            PrintEntry(low, high, type, state, attr, rwx);

            low = high + 1;

            type = cur->GetType();
            state = cur->GetState();
            attr = cur->GetAttribute();
            rwx = cur->GetOwnerRights();
        }
    }

    high = GetAddress(cur) - 1;
    PrintEntry(low, high, type, state, attr, rwx);
}

void
PageFrameTable::PrintFreeArea()
{
    System.Print("-- print free regions: %p, %.8lu\n", _table, _count);
    for (L4_Word_t cur = 0; cur < _count; cur++) {
        if (_table[cur].GetState() == PAGE_STATE_FREE) {
            L4_Word_t   start, end;

            start = pfn2addr(cur);
            cur++;
            while (_table[cur].GetState() == PAGE_STATE_FREE) {
                cur++;
                if (cur > _count) {
                    return;
                }
            }
            end = pfn2addr(cur);
            System.Print("%.8lX - %.8lX\n", start, end);
        }
    }
}

void
PageFrameTable::PrintMemoryUsage()
{
    UInt    free_counter = 0;
    UInt    alloc_counter = 0;
    UInt    map_counter = 0;
    UInt    unmap_counter = 0;
    UInt    unknown_counter = 0;
    UInt    system_counter = 0;

    for (PageFrame* cur = _table; cur < _table + _count; cur++) {
        if (cur->GetType() == PAGE_TYPE_CONVENTIONAL) {
            switch (cur->GetState()) {
                case PAGE_STATE_FREE:
                    free_counter++;
                    break;
                case PAGE_STATE_ALLOC:
                    alloc_counter++;
                    break;
                case PAGE_STATE_MAP:
                    map_counter++;
                    break;
                case PAGE_STATE_UNMAP:
                    unmap_counter++;
                    break;
                default:
                    unknown_counter++;
                    break;
            }
        }
        else {
            system_counter++;
        }
    }

    System.Print(System.INFO, "-- memory usage\n"
               "    free    alloc   mapped unmapped  unknown   system\n"
               "%8lu %8lu %8lu %8lu %8lu %8lu\n",
               free_counter, alloc_counter, map_counter, unmap_counter,
               unknown_counter, system_counter);
}

//-----------------------------------------------------------------------------
//      PFT Initialization Routines
//-----------------------------------------------------------------------------

static void ProbeConventionalMem(addr_t *base, L4_Word_t *length);
static PageFrame *AllocatePageFrameTable(addr_t base, L4_Word_t length,
                                         L4_Word_t size);
static void InitializeMainPft(PageFrame *table, addr_t base,
                              L4_Word_t count);
static void InitializeIoPft(PageFrame *table, L4_Word_t count);

static inline UInt
addr2pfn(addr_t base, addr_t addr)
{
    return (addr - base) >> PAGE_BITS;
}

static inline addr_t
pfn2addr(addr_t base, UInt pfn)
{
    return base + (pfn << PAGE_BITS);
}



///
/// Creates the page frame table for the main memory.
/// @param table Receives the table address.
/// @param base  Receives the base address of the conventional memory (page
///              aligned).
/// @param cnt   Receives the number of pages in conventional memory, starting
///              from base.
///
void
CreateMainPft(PageFrameTable& pft)
{
    L4_Word_t       base;
    L4_Word_t       length; // in bytes
    L4_Word_t       size;   // in bytes
    L4_Word_t       count;  // in pages
    PageFrame       *frame;

    ENTER;

    //
    // Get free area
    //
    ProbeConventionalMem(&base, &length);

    System.Print("conventional mem base:%.8lX length:%.8lX\n", base, length);

    //
    // Number of pages in the area
    count = length >> PAGE_BITS;
    // Size required for the PageFrameTable
#ifdef BUDDY_ALLOCATOR
    size = sizeof(PageFrame) * count + sizeof(BuddyAllocator);
#else
    size = sizeof(PageFrame) * count;
#endif // BUDDY_ALLOCATOR

    // Allocate space for the page frame table at the end of conventional memory
    frame = AllocatePageFrameTable(base, length, size);
    if (frame == 0) FATAL("Could not allocate page frame table!");

    // Set the whole table to zero to avoid uninitialized members
    memset(frame, 0, size);
    System.Print("PFT @ %p 0x%lX bytes 0x%lX entries\n",
         frame, sizeof(PageFrame) * count, count);

    //
    // Set up the initial properties of the page frames.
    //
    InitializeMainPft(frame, base, count);

    pft.Initialize(frame, base, count);

    EXIT;
}

///
/// Get the base address and the length of the conventional memory region.
/// The region include reserved and/or architecture-specific regions at this
/// moment.
///
/// @param base     the base address of the conventional memory region will be
///                  written here.
/// @param length   the size of the conventional memory region in byte will be
///                  written here.
///
static void
ProbeConventionalMem(addr_t *base, L4_Word_t *length)
{
    L4_KernelInterfacePage_t    *kip;
    
    L4_Word_t   start = ~0;
    L4_Word_t   end = 0;

    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();

    for (L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
        L4_MemoryDesc_t *desc = L4_MemoryDesc(kip, i);
        if (L4_IsMemoryDescVirtual(desc)) {
            continue;
        }

        if (L4_ConventionalMemoryType == (L4_MemoryDescType(desc) & 0xF)) {
            start = min(start, L4_MemoryDescLow(desc));
            end = max(end, L4_MemoryDescHigh(desc));
        }
    }

    *base = start & PAGE_MASK;
    *length = PAGE_ALIGN(end) - *base;
}

///
/// Allocate the end of non-reserved conventional memory to store
/// the page frame table.
/// @param base    Beginning address of the conventional memory
/// @param length  Size (in bytes) of the conventional memory
/// @param size    Size (in bytes) to allocate for the page frame table
/// @return        The highest address of non-reserved conventional
///                memory that can hold size bytes, page-aligned.
///                Returns 0 if unable to allocate
///
static PageFrame * 
AllocatePageFrameTable(addr_t base, L4_Word_t length, L4_Word_t size)
{
    ENTER;

    L4_KernelInterfacePage_t *kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();
    // Let's start by trying to use the highest possible address
    L4_Word_t max = (base + length - size) & PAGE_MASK;

    // Now we will check whether the candidate area overlaps with some
    // reserved memory. It this is the case, the area location is decreased
    // out of the reserved memory and we check again there is no overlap.
    Bool overlap;
    do {
        overlap = FALSE;
        for (L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
            L4_MemoryDesc_t *desc = L4_MemoryDesc(kip, i);
	    L4_Word_t type = L4_MemoryDescType(desc) & 0xF;
	    // Just ignore virtual memory descriptors
	    if (L4_IsMemoryDescVirtual(desc)) continue;
    
            switch (type) {
	        // Reserved memory
	        case L4_ReservedMemoryType:
		case L4_ArchitectureSpecificMemoryType:
		case L4_BootLoaderSpecificMemoryType:
                {
		    L4_Word_t start = L4_MemoryDescLow(desc);
		    L4_Word_t end = L4_MemoryDescHigh(desc);
		    // Overlap?
		    if (max >= start && max < end) {
		        overlap = TRUE;
			max = (start - size) & PAGE_MASK;
	            }
		    break;
                }
		// Available memory
	        default:
		    continue;
	    }
        }
    } while (overlap);

    EXIT;

    // Return the area the computed. In any case, the first megabyte of memory
    // is reserved, so return an error if we couldn't allocate above it.
    return (PageFrame *) (max >= 0x100000 ? max : 0L);
}

///
/// Initializes the page frame table of the main memory.
///
/// @param table    the page frame table to be initialized
/// @param base     the base address of the managed area
/// @param count    the number of pages to be managed
///
static void
InitializeMainPft(PageFrame *table, addr_t base, L4_Word_t count)
{
    L4_KernelInterfacePage_t    *kip;
    L4_Word_t   index, length;

    ENTER;

    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();

    for (L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
        L4_MemoryDesc_t *desc;
        L4_Word_t       type, low, high, state, rwx;
        
        desc = L4_MemoryDesc(kip, i);
        if (L4_IsMemoryDescVirtual(desc)) {
            continue;
        }

        type = L4_MemoryDescType(desc) & 0xF;
        low = L4_MemoryDescLow(desc);
        high = L4_MemoryDescHigh(desc);

        switch (type) {
            case L4_ReservedMemoryType: {
                state = PAGE_STATE_FREE;
                rwx = PAGE_PERM_NONE;
                break;
            }
            case L4_ArchitectureSpecificMemoryType: {
                state = PAGE_STATE_FREE;
                rwx = PAGE_PERM_FULL;
                break;
            }
            case L4_BootLoaderSpecificMemoryType: {
                state = PAGE_STATE_FREE;
                rwx = PAGE_PERM_FULL;
                break;
            }
            case L4_ConventionalMemoryType: {
                // Reserve the first 1MB region
                if (high < 0x100000) {
                    type = PAGE_TYPE_RESERVED;
                    state = PAGE_STATE_FREE;
                    rwx = PAGE_PERM_FULL;
                }
                else {
                    // Set the state in-use for the initialization of
                    // page allocator.
                    state = PAGE_STATE_ALLOC;
                    rwx = PAGE_PERM_FULL;
                }
                break;
            }
            default:
                continue;
        }

        for (L4_Word_t j = addr2pfn(base, low);
             j < addr2pfn(base, high) + 1;
             j++) {
            table[j].SetPageGroup(1);
            table[j].SetState(state);
            table[j].SetType(type);
            table[j].SetOwnerRights(rwx);
            table[j].SetAttribute(0);
        }
    }

    //FIXME: XXX: Hack: Exclude the memory areas for Sigma0 and the root task
    index = addr2pfn(base, PhysLayout::SIGMA0_START);
    length = 5;
    for (L4_Word_t i = index; i < index + length; i++) {
        table[i].SetState(PAGE_STATE_ALLOC);
        table[i].SetType(PAGE_TYPE_RESERVED);
        table[i].SetOwner(L4_Myself());
        table[i].SetOwnerRights(PAGE_PERM_READ);
        table[i].SetSharerRights(PAGE_PERM_NONE);
        table[i].SetAttribute(0);
        table[i].SetPageGroup(1);
    }

    index = addr2pfn(base, PhysLayout::ROOTTASK_START);
    length = 0x18;
    for (L4_Word_t i = index; i < index + length; i++) {
        table[i].SetState(PAGE_STATE_ALLOC);
        table[i].SetType(PAGE_TYPE_RESERVED);
        table[i].SetOwner(L4_Myself());
        table[i].SetOwnerRights(PAGE_PERM_READ);
        table[i].SetSharerRights(PAGE_PERM_NONE);
        table[i].SetAttribute(0);
        table[i].SetPageGroup(1);
    }

    //
    // Invalidate the pages where the table itself is stored.
    //
    index = addr2pfn(base, (L4_Word_t)table);
    length = (sizeof(PageFrame) * count) >> PAGE_BITS;

    System.Print("pft init: base %.8lX length %lu\n", (L4_Word_t)table, length);

    for (L4_Word_t i = index; i < index + length; i++) {
        table[i].SetState(PAGE_STATE_ALLOC);
        table[i].SetType(PAGE_TYPE_RESERVED);
        table[i].SetOwner(L4_Myself());
        table[i].SetOwnerRights(PAGE_PERM_READ_WRITE);
        table[i].SetSharerRights(PAGE_PERM_NONE);
        table[i].SetAttribute(0);
        table[i].SetPageGroup(1);
    }

    //
    // Invalidate the page where the page allocator is instantiated.
    //
    index = addr2pfn(base, (L4_Word_t)(table + count));
    table[index].SetState(PAGE_STATE_ALLOC);
    table[index].SetType(PAGE_TYPE_RESERVED);
    table[index].SetOwner(L4_Myself());
    table[index].SetOwnerRights(PAGE_PERM_READ_WRITE);
    table[index].SetSharerRights(PAGE_PERM_NONE);
    table[index].SetAttribute(0);
    table[index].SetPageGroup(1);

    EXIT;
}

PageFrameTable *
CreateIoPft()
{
    L4_KernelInterfacePage_t    *kip;
    PageFrame       *table;
    PageFrameTable  *obj;
    L4_Word_t       pages;
    L4_Word_t       size;
    L4_Word_t       start;
    L4_Word_t       end;

    ENTER;

    // Look for the end of the conventional memory

    ENTER;

    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();

    start = 0;
    for (L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
        L4_MemoryDesc_t*    desc;
        L4_Word_t           type, high;
        
        desc = L4_MemoryDesc(kip, i);
        if (L4_IsMemoryDescVirtual(desc)) {
            continue;
        }

        type = L4_MemoryDescType(desc) & 0xF;
        if (type != L4_ConventionalMemoryType) {
            continue;
        }

        high = L4_MemoryDescHigh(desc);
        start = high < start ? start : high;
    }

    start = PAGE_ALIGN(start);

    // Give some amount of space
    end = start + 0x10000000;
    if (PhysLayout::ROOT_UTCB_START < end) {
        end = PhysLayout::ROOT_UTCB_START;
    }

    // Number of IO mapped pages
    pages = (end - start) >> PAGE_BITS;
    // Size required to keep information for all pages
    size = pages * sizeof(PageFrame);

    MainPa.Allocate(size >> PAGE_BITS, reinterpret_cast<L4_Word_t *>(&table));
    // Set the whole table to zero to avoid uninitialized members
    memset(table, 0, size);
    obj = (PageFrameTable *)malloc(sizeof(PageFrameTable));
    if (obj == 0) {
        FATAL("CreateIoPft");
        return 0;
    }

    obj->Initialize(table, start, pages);
    InitializeIoPft(table, pages);

    EXIT;
    return obj;
}

static void
InitializeIoPft(PageFrame *table, L4_Word_t count)
{
    for (L4_Word_t i = 0; i < count; i++) {
        table[i].SetType(PAGE_TYPE_CONVENTIONAL);
        table[i].SetState(PAGE_STATE_ALLOC);
        table[i].SetOwner(L4_Myself());
        table[i].SetOwnerRights(PAGE_PERM_READ_WRITE);
        table[i].SetPageGroup(1);
    }
}

