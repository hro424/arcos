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
/// @file   Services/Root/Space.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: Space.h 377 2008-08-08 07:53:33Z hro $

#ifndef ARC_ROOT_SPACE_H
#define ARC_ROOT_SPACE_H

#include <Bitmap.h>
#include <BPlusTree.h>
#include <List.h>
#include <MapElement.h>
#include <Types.h>
#include <sys/Config.h>
#include <l4/types.h>
#include "SnapshotStore.h"

typedef MapElement<addr_t, PageFrame*>  MapListElement_t;
typedef List<MapListElement_t*>         MapList_t;

class PageFrame;
struct Thread;

struct ThreadContext
{
    addr_t  ip;
    addr_t  sp;
};

class Space {
public:
    ///
    /// Linked list for management
    ///
    Space*                  next;

private:
    static Bitmap*          _tid_map;

    ///
    /// Mutex fot the thread ID bitmap
    ///
    static Mutex            _tid_lock;

    ///
    /// The base number of user available thread ID
    ///
    static L4_Word_t        _tid_base;

    ///
    /// If this space is shadow task or not
    ///
    Bool                    _shadow;

    ///
    /// Pager of the space
    ///
    L4_ThreadId_t           _pager;

    ///
    /// The space specifier, or the root thread
    ///
    Thread*                 _root;

    ///
    /// Threads in the space
    ///
    List<Thread*>           _residents;

    ///
    /// The KIP area of this address space
    ///
    L4_Fpage_t              _kip_area;

    ///
    /// The user thread control block area of this address space.  Multiple
    /// thread control blocks are stored there.
    ///
    L4_Fpage_t              _utcb_area;

    ///
    /// The size of a user thread control block
    ///
    L4_Word_t               _utcb_size;

    ///
    /// The bitmap to manage the UTCB slots
    ///
    Bitmap*                 _utcb_map;

    ///
    /// Memory mapping database
    ///
    BPlusTree<addr_t, PageFrame*, 31>
                            _map_db;

    ///
    /// Snapshot repository
    ///
    SnapshotStore<MapList_t*, 10>       _snapshots;

    SnapshotStore<ThreadContext*, 4>    _thread_context;

    void ReleaseOldFrames(UInt gen, MapList_t* ls);

    Space() {}

    ///
    /// Allocates a UTCB.
    ///
    /// @return the virtual address of the UTCB
    ///
    L4_Word_t AllocateUtcb();

    ///
    /// Releases the specified UTCB.
    ///
    /// @param utcb     the UTCB
    ///
    void ReleaseUtcb(L4_Word_t utcb);

    L4_ThreadId_t AllocateThreadId();

    void ReleaseThreadId(L4_ThreadId_t tid);

    ///
    /// Adds the thread object that runs in this address space to the list.
    ///
    /// @param th       the thread object
    ///
    void AddResident(Thread* th);

    ///
    /// Removes the thread object from the list.
    ///
    /// @param th       the thread object
    ///
    void DeleteResident(Thread* th);

    ///
    /// Create a new thread object.
    ///
    stat_t CreateThread0(Thread** th);

public:
    static void InitializeTidMap();

    ///
    /// Initializes the address space object.
    ///
    /// @param kip      the KIP area
    /// @param utcb     the UTCB area
    /// @param usize    the size of a UTCB
    /// @param shadow   if this space is shadow task or not
    ///
    Space(L4_Fpage_t kip, L4_Fpage_t utcb, L4_Word_t usize, Bool shadow);

    ~Space();

    L4_ThreadId_t GetPager();

    Bool IsShadow();

    const Thread* GetRootThread();

    void SetPager(L4_ThreadId_t tid);

    const L4_Fpage_t& UtcbArea();

    const L4_Fpage_t& KipArea();

    ///
    /// Creates a new thread object and adds it to the resident list.
    /// Note, this doesn't actually create nor schedule the L4 thread.
    ///
    /// @param thread       the thread object
    ///
    stat_t CreateThread(Thread** thread);

    ///
    /// Deletes the thread object.  Note, this doesn't actually stops the
    /// thread.
    ///
    /// @param thread       the thread object
    ///
    void DeleteThread(Thread* thread);

    ///
    /// Searches the thread object with the given thread ID.
    ///
    /// @param tid          the thread ID for the key to search
    /// @param thread       the thread object
    ///
    stat_t FindThread(L4_ThreadId_t tid, Thread** thread);

    ///
    /// Registers the mapping of pages.
    ///
    /// @param address      the virtual address of the mapping destination
    /// @param frame        the page frame that is the mapping source
    ///
    void InsertMap(L4_Word_t address, PageFrame *frame);

    ///
    /// Registers the mapping of pages (fancy version)
    ///
    /// @param address      the virtual address of the mapping destination
    /// @param frame        the page frame that is the mapping source
    /// @param rwx          the permission of the pages
    /// @param attr         the attribute of the pages
    /// @param peer         the page sharing partner
    ///
    void InsertMap(L4_Word_t address, PageFrame *frame, L4_Word_t rwx,
                   L4_Word_t attr, L4_ThreadId_t peer);

    ///
    /// Registers the mapping of pages.  This method overwrites an entry if
    /// it is already registered in the database.
    ///
    Bool SetMap(L4_Word_t address, PageFrame *frame);

    ///
    /// Removes the mapping information from the database.
    ///
    /// @param address      the mapping destination
    ///
    void RemoveMap(L4_Word_t address);

    ///
    /// Searches the mapping in the database.
    ///
    /// @param address      the mapping destination
    /// @param frame        the page frame where the destination is mapped
    ///
    stat_t SearchMap(L4_Word_t address, PageFrame** frame);

    ///
    /// Takes a snapshot of the mapping database.
    ///
    stat_t Snapshot(addr_t ip, addr_t sp);

    ///
    /// Restore a snapshot of the mapping database.
    ///
    stat_t Restore(UInt generation, addr_t *ip, addr_t *sp);

    void DumpMapDB();
    void DumpMap();
};

inline Bool
Space::IsShadow()
{
    return _shadow;
}

inline L4_ThreadId_t
Space::GetPager()
{
    return _pager;
}

inline const Thread*
Space::GetRootThread()
{
    return _root;
}

inline void
Space::SetPager(L4_ThreadId_t tid)
{
    _pager = tid;
}

inline const L4_Fpage_t&
Space::UtcbArea()
{
    return _utcb_area;
}

inline const L4_Fpage_t&
Space::KipArea()
{
    return _kip_area;
}


#endif // ARC_ROOT_SPACE_H

