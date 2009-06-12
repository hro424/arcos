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
/// @file   Services/Root/PageFrame.h
/// @brief  Page Frame Object
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: PageFrame.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_PAGE_FRAME_H
#define ARC_PAGE_FRAME_H

#include <Types.h>
#include <sys/Config.h>
#include <l4/kip.h>
#include <l4/types.h>


enum PageType {
    PAGE_TYPE_UNDEFINED =       L4_UndefinedMemoryType,
    PAGE_TYPE_CONVENTIONAL =    L4_ConventionalMemoryType,
    PAGE_TYPE_RESERVED =        L4_ReservedMemoryType,
    PAGE_TYPE_SHARED =          L4_SharedMemoryType,
    PAGE_TYPE_BOOT =            L4_BootLoaderSpecificMemoryType,
};

#define PAGE_STATE_MASK         0xF0
#define PAGE_STATE_FREE         0x10
#define PAGE_STATE_UNMAP        0x20
#define PAGE_STATE_ALLOC        0x30
#define PAGE_STATE_MAP          0x40
#define PAGE_STATE_READ         0x01
#define PAGE_STATE_WRITE        0x02
#define PAGE_STATE_EXEC         0x04

#define PAGE_ATTR_BITS          14
#define PAGE_ATTR_MASK          ((1 << PAGE_ATTR_BITS) - 1)
#define PAGE_ATTR_EMPTY         0x001
#define PAGE_ATTR_CONST         0x002
#define PAGE_ATTR_COW           0x004       // Copy on write page
#define PAGE_ATTR_SNAPSHOT      0x008

#define PAGE_PERM_MASK          0x7
#define PAGE_PERM_READ          L4_Readable
#define PAGE_PERM_WRITE         L4_Writable
#define PAGE_PERM_EXEC          L4_eXecutable
#define PAGE_PERM_FULL          L4_FullyAccessible
#define PAGE_PERM_READ_WRITE    L4_ReadWriteOnly
#define PAGE_PERM_READ_EXEC     L4_ReadeXecOnly
#define PAGE_PERM_NONE          L4_NoAccess

#define IS_READABLE(rwx)        (((rwx) & PAGE_PERM_READ) == PAGE_PERM_READ)
#define IS_WRITABLE(rwx)        (((rwx) & PAGE_PERM_WRITE) == PAGE_PERM_WRITE)
#define IS_EXECUTABLE(rwx)      (((rwx) & PAGE_PERM_EXEC) == PAGE_PERM_EXEC)


///
/// Keep various information about a memory page. Main component of low-level
/// memory management along with PageFrameTable.
///
class PageFrame {
public:
    static const UInt   MAX_GENERATION = 10;

    ///
    /// Used by free-page management
    ///
    PageFrame           *next;

    UShort              RefCnt;

private:
    ///
    /// Mapping destination
    ///
    addr_t              _destination;

    ///
    /// Owner of the page
    ///
    L4_ThreadId_t       _owner;

    ///
    /// Threads that is allowed to sharer this page
    ///
    L4_ThreadId_t       _sharer;

    ///
    /// Permission to the owner
    ///
    L4_Word_t           _owner_rights:4;

    ///
    /// Permission to the sharers
    ///
    L4_Word_t           _sharer_rights:4;

    ///
    /// State of the page
    ///
    UByte               _state;

    ///
    /// The generation of the page.  Used by snapshotting.
    ///
    UByte               _generation;

    ///
    /// Attribute of the page
    ///
    UByte               _attribute;

    ///
    /// Length of the frame (by PAGE_SIZE units)
    /// A length of x means that this page is part of a block 
    /// of x pages.
    ///
    L4_Word_t           _group;

    ///
    /// Type of the page
    ///
    UByte               _type;

public:
    void Initialize();

    addr_t GetDestination() const;
    void SetDestination(addr_t addr);

    L4_ThreadId_t GetOwner() const;
    void SetOwner(L4_ThreadId_t tid);
    L4_Word_t GetOwnerRights() const;
    void SetOwnerRights(L4_Word_t rwx);

    L4_ThreadId_t GetSharer() const;
    void SetSharer(L4_ThreadId_t tid);
    L4_Word_t GetSharerRights() const;
    void SetSharerRights(L4_Word_t rwx);

    UByte GetState() const;
    void SetState(UByte s);

    UByte GetAccessState() const;
    void SetAccessState(UByte s);
    void AddAccessState(UByte s);
    Bool IsAccessed(UByte perm) const {
        return (_state & perm) == perm;
    }
    
    UByte GetAttribute() const;
    void SetAttribute(UByte attr);

    L4_Word_t GetPageGroup() const;
    void SetPageGroup(L4_Word_t group);

    UByte GetType() const;
    void SetType(UByte type);

    UByte GetGeneration() const;
    void SetGeneration(UByte gen);
    void IncrementGeneration();
    void DecrementGeneration();

    Bool IsShared() const { return _sharer_rights > 0; }

    Bool IsCOW() const {
        return ((_attribute & PAGE_ATTR_COW) != 0);
    }

    Bool IsSnapshot() const {
        return ((_attribute & PAGE_ATTR_SNAPSHOT) != 0);
    }

    PageFrame &operator=(const PageFrame &frame);
};

inline void
PageFrame::Initialize()
{
    _destination = 0;
    _owner = L4_nilthread;
    _sharer = L4_nilthread;
    _owner_rights = 0;
    _sharer_rights = 0;
    _state = PAGE_STATE_FREE;
    _generation = 0;
    _attribute = 0;
}

inline addr_t
PageFrame::GetDestination() const
{
    return _destination;
}

inline void
PageFrame::SetDestination(addr_t addr)
{
    _destination = addr;
}

inline L4_ThreadId_t
PageFrame::GetOwner() const
{
    return _owner;
}

inline void
PageFrame::SetOwner(L4_ThreadId_t tid)
{
    _owner = tid;
}

inline L4_Word_t
PageFrame::GetOwnerRights() const
{
    return _owner_rights;
}

inline void
PageFrame::SetOwnerRights(L4_Word_t rwx)
{
    _owner_rights = rwx & PAGE_PERM_MASK;
}

inline L4_ThreadId_t
PageFrame::GetSharer() const
{
    return _sharer;
}

inline void
PageFrame::SetSharer(L4_ThreadId_t tid)
{
    _sharer = tid;
}

inline L4_Word_t
PageFrame::GetSharerRights() const
{
    return _sharer_rights;
}

inline void
PageFrame::SetSharerRights(L4_Word_t rwx)
{
    _sharer_rights = rwx & PAGE_PERM_MASK;
}

inline UByte
PageFrame::GetState() const
{
    return (_state & PAGE_STATE_MASK);
}

inline void
PageFrame::SetState(UByte s)
{
    _state = (_state & ~PAGE_STATE_MASK) | (s & PAGE_STATE_MASK);
}

inline UByte
PageFrame::GetAccessState() const
{
    return (_state & 0xF);
}

inline void
PageFrame::SetAccessState(UByte s)
{
    _state = (_state & PAGE_STATE_MASK) | (s & 0xF);
}

inline void
PageFrame::AddAccessState(UByte s)
{
    _state = _state | (s & 0xF);
}

inline UByte
PageFrame::GetAttribute() const
{
    return _attribute;
}

inline void
PageFrame::SetAttribute(UByte attr)
{
    _attribute = attr;
}

inline L4_Word_t
PageFrame::GetPageGroup() const
{
    return _group;
}

inline void
PageFrame::SetPageGroup(L4_Word_t group)
{
    _group = group;
}

inline UByte
PageFrame::GetType() const
{
    return _type;
}

inline void
PageFrame::SetType(UByte type)
{
    _type = type;
}

inline UByte
PageFrame::GetGeneration() const
{
    return _generation;
}

inline void
PageFrame::SetGeneration(UByte g)
{
    _generation = g;
}

inline void
PageFrame::IncrementGeneration()
{
    if (_generation < MAX_GENERATION) {
        _generation++;
    }
}

inline void
PageFrame::DecrementGeneration()
{
    if (_generation > 0) {
        _generation--;
    }
}

inline PageFrame &
PageFrame::operator=(const PageFrame &frame)
{
    _destination = frame._destination;
    _owner = frame._owner;
    _owner_rights = frame._owner_rights;
    _sharer = frame._sharer;
    _sharer_rights = frame._sharer_rights;
    _attribute = frame._attribute;

    return *this;
}

#endif // ARC_PAGE_FRAME_H

