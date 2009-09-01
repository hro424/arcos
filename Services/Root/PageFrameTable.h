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
/// @file   Services/Root/PageFrameTable.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: PageFrameTable.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_ROOT_PAGE_FRAME_TABLE_H
#define ARC_ROOT_PAGE_FRAME_TABLE_H

#include <Assert.h>
#include <Debug.h>
#include <Types.h>
#include <sys/Config.h>

#include "PageFrame.h"

///
/// Handles a memory region by dividing it into pages and keeping information
/// about every handled page, such as its availability or is mapping.
/// 
class PageFrameTable {
private:
    ///
    /// An array of PageFrame
    ///
    PageFrame*  _table;

    ///
    /// For boundary checking
    ///
    PageFrame*  _table_end;

    ///
    /// Base address of managed area
    ///
    addr_t      _base;

    ///
    /// Count of managed pages
    ///
    size_t      _count;

    ///
    /// Converts the address to the corresponding page frame number
    ///
    /// @param address  the address to be converted
    /// @return         the page frame number corresponding to the address
    ///
    UInt addr2pfn(addr_t address);

    ///
    /// Converts the page frame number into the corresponding address
    ///
    /// @param pfn      the page frame number
    /// @return         the address of the page
    ///
    addr_t pfn2addr(UInt pfn);

public:

    ///
    /// Initializes the page frame table object.
    ///
    /// @param table        the array of page frames
    /// @param base         the base address of the managed area
    /// @param count        the count of pages
    ///
    void Initialize(PageFrame *table, L4_Word_t base, L4_Word_t count);

    PageFrame *Table() { return _table; }

    ///
    /// Obtains the i-th page frame.
    ///
    /// @param i            the page frame number
    /// @return             the page frame object
    ///
    //PageFrame &operator[](UInt pfn);

    ///
    /// Obtains the page frame corresponding the given address.
    ///
    /// @param address      the address of the page
    /// @return             the page frame object
    ///
    PageFrame *GetFrame(addr_t address);

    ///
    /// Obtains the index of the specified frame in the page frame table.
    ///
    UInt GetFrameIndex(PageFrame *frame);

    ///
    /// Obtains the address of the i-th page.
    ///
    /// @param i            the page frame number
    /// @return             the address
    ///
    addr_t GetAddress(UInt i);

    ///
    /// Obtains the address of the page managed by the frame.
    ///
    /// @param frame        the page frame object
    /// @return             the address
    ///
    addr_t GetAddress(PageFrame *frame);

    ///
    /// Obtains the count of page table entries.
    ///
    /// @return the count of page table entries
    ///
    size_t Length() { return _count; }

    Bool IsValidFrame(PageFrame *f) {
        return (_table <= f && f < _table_end);
    }

    size_t FreeCount()
    {
        size_t free_counter = 0;
        for (PageFrame* cur = _table; cur < _table + _count; cur++) {
            if (cur->GetType() == PAGE_TYPE_CONVENTIONAL) {
                if (cur->GetState() == PAGE_STATE_FREE) {
                    free_counter++;
                }
            }
        }
        return free_counter;
    }

    void PrintMemoryUsage();
    void PrintFreeArea();
    void Dump();
};

inline UInt
PageFrameTable::addr2pfn(addr_t addr)
{
    return (addr - _base) >> PAGE_BITS;
}

inline addr_t
PageFrameTable::pfn2addr(UInt pfn)
{
    return _base + (pfn << PAGE_BITS);
}


inline void
PageFrameTable::Initialize(PageFrame *table, L4_Word_t base, L4_Word_t count)
{
    assert(table != 0);

    _table = table;
    _table_end = table + count;
    _base = base;
    _count = count;

    DOUT("Page frame table (%p) %p - %p (base: %.8lX  %lu pages)\n",
         this, _table, _table_end, _base, _count);
}

inline PageFrame *
PageFrameTable::GetFrame(L4_Word_t address)
{
    if (_base <= address && address < _base + PAGE_SIZE * _count) {
        return _table + addr2pfn(address);
    }
    else {
        return 0;
    }
}

inline UInt
PageFrameTable::GetFrameIndex(PageFrame *frame)
{
    assert(_table <= frame);
    assert(frame < _table_end);

    if (_table <= frame && frame < _table_end) {
        return frame - _table;
    }
    else {
        return ~0UL;
    }
}

inline addr_t
PageFrameTable::GetAddress(PageFrame *frame)
{
    if (_table <= frame && frame < _table_end) {
        return pfn2addr((L4_Word_t)(frame - _table));
    }
    else {
        return ~0UL;
    }
}

///
/// Creates the main page frame table.
///
void CreateMainPft(PageFrameTable& pft);

///
/// Creates a page frame table for mapped I/O.
///
PageFrameTable *CreateIoPft();

#endif // ARC_ROOT_PAGE_FRAME_TABLE_H

