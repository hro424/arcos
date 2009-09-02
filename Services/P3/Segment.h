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
/// @breif  A template for address space segments
/// @file   Services/P3/Segment.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///


#ifndef ARC_PEL2_SEGMENT_H
#define ARC_PEL2_SEGMENT_H

#include <Debug.h>
#include <MemoryManager.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>
#include <l4/types.h>
#include <l4/thread.h>

///
/// Segment represents a memory region in a virtual address space.
///
class Segment
{
protected:
    addr_t      _start;
    addr_t      _end;

public:
    ///
    /// Initializes the segment with its start and end address.  They should
    /// be aligned in page size.  The end address is excluded of the segment.
    ///
    /// @param start    the start address of the segment
    /// @param end      the end address of the segment
    ///
    virtual void Initialize(addr_t start, addr_t end)
    {
        _start = start;
        _end = end;
        DOUT("segment: %.8lX - %.8lX\n", start, end);
    }

    ///
    /// Handles the page fault in this segment.
    ///
    /// @param tid      the thread that has generated the page fault
    /// @param faddr    the fault address
    /// @param fip      the fault instruction pointer
    /// @param rwx      the fault access rights
    /// @return         the status of the procedure
    ///
    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx) = 0;

    virtual void Release()
    {
        stat_t err;
        for (addr_t addr = _start; addr < _end; addr += PAGE_SIZE) {
            err = Pager.Release(addr);
        }
    }


    ///
    /// Tests if the given address is within the segment.
    ///
    /// @param address      the address to be tested
    /// @return             the result of the test
    ///
    Bool Hit(addr_t address) { return (_start <= address) && (address < _end); }

    ///
    /// Obtains the start address of the segment.
    ///
    /// @return     the start address
    ///
    addr_t StartAddress() { return _start; }

    ///
    /// Obtains the end address of the segment.
    ///
    /// @return     the end address
    ///
    addr_t EndAddress() { return _end; }
};

///
/// Growing segment is a memory region which size is increasing at runtime.
///
class GrowingSegment : public Segment
{
protected:
    addr_t  _cursor;
public:
    virtual void Initialize(addr_t start) = 0;
};

///
/// User programs are prohibited to access to the memory area below its text
/// area.  The head segment is a guard for that area.
///
class Segment_Head : public Segment
{
public:
    virtual void Initialize(addr_t start = 0, addr_t end = 0)
    {
        _start = 0;
        //_end = VirtLayout::USER_TEXT_START;
        _end = VirtLayout::SHM_START;
        DOUT("head segment: %.8lX - %.8lX\n", start, end);
    }


    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);
};

///
/// User programs are prohibited to access to the memory area above its stack
/// area.  The end segment is a guard for that area.
///
class Segment_Tail : public Segment
{
public:
    virtual void Initialize(addr_t start, addr_t end)
    {
        _start = VirtLayout::USER_STACK_END;
        _end = ~0UL;
        DOUT("tail segment: %.8lX - %.8lX\n", start, end);
    }


    void Initialize()
    {
        this->Initialize(0, 0);
    }


    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx)
    {
        System.Print(System.ERROR, "[%.8lX] Access violation.\n",
                     L4_Myself().raw);
        System.Print(System.ERROR,
                     "virt %.8lX, ip %.8lX, rwx: %lX, from %.8lX\n",
                     faddr, fip, *rwx, tid.raw);
        *rwx = L4_NoAccess;
        return ERR_INVALID_RIGHTS;
    }

};

class TextSegment : public Segment
{
public:
    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);
};

class DataSegment : public Segment
{
public:
    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);
};

///
/// The persistent segment deals with the access to the persistent memory
/// area.
///
class PersistentSegment : public Segment
{
public:
    virtual void Initialize(addr_t start, addr_t end);
    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);
};

///
/// The heap segment deals with the access to the heap area.
///
class HeapSegment : public GrowingSegment
{
public:
    virtual void Initialize(addr_t start)
    {
        _start = start;
        _end = _start + PAGE_SIZE;
        _cursor = _end;
        DOUT("heap segment: %.8lX - %.8lX\n", _start, _end);
    }

    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);

    ///
    /// Expands the segment.
    ///
    /// @return     the address of the expanded page
    ///
    virtual addr_t Grow(size_t count)
    {
        addr_t newp = _end;

        _end += PAGE_SIZE * count;
        _cursor += PAGE_SIZE * count;

        DOUT("heap grow: %.8lX -> %.8lX\n", newp, _end);
        return newp;
    }


};

///
/// The stack segment deals with the access to the stack area.
///
class StackSegment : public GrowingSegment
{
protected:
    addr_t Grow()
    {
        addr_t  newp = _start;
        _start -= PAGE_SIZE;
        _cursor -= PAGE_SIZE;
        return newp;
    }


public:
    virtual void Initialize(addr_t start)
    {
        _start = start - PAGE_SIZE;
        _end = start;
        _cursor = _end;
        DOUT("stack segment: %.8lX - %.8lX\n", _start, _end);
    }


    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t *rwx);

    //virtual addr_t SetupArgs(L4_Word_t *args, size_t count);
    virtual addr_t SetupArgs(addr_t heap, Int argc, char* argv[], Int state);
};


class SessionSegment : public Segment
{
public:
    virtual void Initialize(addr_t start, addr_t end)
    {
        _start = start;
        _end = end;
        DOUT("session segment: %.8lX - %.8lX\n", start, end);
    }

    virtual stat_t HandlePf(L4_ThreadId_t tid, L4_Word_t faddr,
                            L4_Word_t fip, L4_Word_t* rwx)
    {
        DOUT("session segment\n");
        return Pager.Map(faddr, L4_ReadWriteOnly);
    }

};


#endif // ARC_PEL2_SEGMENT_H

