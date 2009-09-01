/*
 *
 *  Copyright (C) 2006, 2007, 2008, 2009, Waseda University.
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
/// @brief  The implementation of the address space object
/// @file   Services/Root/Space.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///


#include <Debug.h>
#include <Mutex.h>
#include <Bitmap.h>
#include <BPlusTree.h>
#include <List.h>
#include <System.h>
#include <String.h>
#include <Types.h>
#include <sys/Config.h>

#include "Common.h"
#include "PageAllocator.h"
#include "PageFrame.h"
#include "Pager.h"
#include "SnapshotStore.h"
#include "Space.h"
#include "Task.h"
#include "Thread.h"

#include <l4/types.h>
#include <l4/thread.h>
#include <l4/kip.h>


extern Pager    Pg;

Bitmap*     Space::_tid_map;
Mutex       Space::_tid_lock;
L4_Word_t   Space::_tid_base;

///
/// Initializes the thread ID bitmap
///
void
Space::InitializeTidMap()
{
    L4_KernelInterfacePage_t*   kip =
        static_cast<L4_KernelInterfacePage_t*>(L4_GetKernelInterface());

    _tid_base = L4_ThreadIdUserBase(kip) + Thread::TID_OFFSET;
    _tid_map = new Bitmap(Config::MAX_GLOBAL_THREADS);
    _tid_map->Reset();
}

Space::Space(L4_Fpage_t kip, L4_Fpage_t utcb, L4_Word_t usize, Bool shadow)
    : _shadow(shadow), _kip_area(kip), _utcb_area(utcb), _utcb_size(usize)
{
    Thread*     root;
    stat_t      err;

    _utcb_map = new Bitmap(Config::MAX_LOCAL_THREADS);
    _utcb_map->Reset();

    if ((err = CreateThread0(&root)) != ERR_NONE) {
        delete _utcb_map;
        FATAL("Space_Ctor");
    }

    _root = root;
    next = 0;
    _pager = L4_nilthread;

    _residents.Append(_root);
    _snapshots.Initialize();
    _thread_context.Initialize();
}


Space::~Space()
{
    DeleteAllThreadObj();
    delete _utcb_map;
}


inline L4_Word_t
Space::AllocateUtcb()
{
    L4_Word_t utcb = 0;
    for (size_t idx = 0; idx < _utcb_map->Length(); ++idx) {
        if (!_utcb_map->Test(idx)) {
            _utcb_map->Set(idx);
            //map = _utcb_map->GetMap();
            utcb = L4_Address(_utcb_area) + idx * _utcb_size;
            break;
        }
    }
    return utcb;
}


inline L4_ThreadId_t
Space::AllocateThreadId()
{
    L4_ThreadId_t   tid = L4_nilthread;

    _tid_lock.Lock();
    for (size_t idx = 0; idx < _tid_map->Length(); ++idx) {
        if (!_tid_map->Test(idx)) {
            _tid_map->Set(idx);
            tid = L4_GlobalId(idx + _tid_base, Thread::TID_INITIAL_VERSION);
            break;
        }
    }
    _tid_lock.Unlock();

    return tid;
}


stat_t
Space::CreateThread0(Thread** thread)
{
    Thread*         th;
    L4_ThreadId_t   tid;
    L4_Word_t       utcb;

    ENTER;

    th = new Thread;

    // Get a global TID
    tid = AllocateThreadId();
    if (L4_IsThreadEqual(tid, L4_nilthread)) {
        delete th;
        return ERR_OUT_OF_RANGE;
    }

    utcb = AllocateUtcb();
    if (utcb == 0) {
        ReleaseThreadId(tid);
        delete th;
        return ERR_OUT_OF_MEMORY;
    }

    th->Id = tid;
    th->Utcb = utcb;
    th->AddressSpace = this;
    th->Irq = 0;

    *thread = th;

    EXIT;
    return ERR_NONE;
}

stat_t
Space::CreateThreadObj(Thread** th)
{
    Thread*     t;
    stat_t    err;

    err = CreateThread0(&t);
    if (err != ERR_NONE) {
        return err;
    }

    _residents.Add(t);
    *th = t;

    return ERR_NONE;
}

//XXX: This makes cleanup very difficult!  Better not to use List<>.
void
Space::DeleteThreadObj(Thread *thread)
{
    if (thread != 0) {
        _residents.Remove(thread);
        ReleaseUtcb(thread->Utcb);
        ReleaseThreadId(thread->Id);
        delete thread;
    }
}

void
Space::DeleteAllThreadObj()
{
    Iterator<Thread*>& it = _residents.GetIterator();

    if (it.HasNext()) {
        Thread* th1 = it.Next();
        while (it.HasNext()) {
            Thread* th2 = it.Next();
            DeleteThreadObj(th1);
            th1 = th2;
        }
        DeleteThreadObj(th1);
    }
}

stat_t
Space::FindThread(L4_ThreadId_t tid, Thread **thread)
{
    Iterator<Thread*>&  it = _residents.GetIterator();
    while (it.HasNext()) {
        Thread* ptr = it.Next();
        if (L4_ThreadNo(ptr->Id) == L4_ThreadNo(tid)) {
            *thread = ptr;
            return ERR_NONE;
        }
    }
    return ERR_NOT_FOUND;
}

///
/// Prepare frame to be mapped into the current space at
/// the given virtual address.
///
void
Space::InsertMap(L4_Word_t address, PageFrame *frame)
{
#if SYS_DEBUG
    PageFrame*  f;
    if (_map_db.Search(address, f)) {
        DOUT("address is already registered: %.8lX\n", address);
        if (f != frame) {
            DOUT("registered value %p is different from %p\n",
                 f, frame);
            BREAK("Map DB inconsistency");
        }
    }
#endif // SYS_DEBUG

    _map_db.Insert(address, frame);

#if SYS_DEBUG
    if (!_map_db.Search(address, f)) {
        BREAK("address is not correctly registered");
    }
#endif // SYS_DEBUG
}

Bool
Space::SetMap(L4_Word_t address, PageFrame *frame)
{
    PageFrame*  dummy;
    Bool        result;
    result = _map_db.Update(address, frame, dummy);
    DOUT("Replace %p with %p @ %.8lX\n", dummy, frame, address);

#if SYS_DEBUG
    PageFrame*  f;
    if (!_map_db.Search(address, f)) {
        BREAK("address is not correctly registered");
    }
    DOUT("registered value %p\n", f);
    if (f != frame) {
        BREAK("Map DB inconsistency");
    }
#endif // SYS_DEBUG

    return result;
}


stat_t
Space::SearchMap(L4_Word_t address, PageFrame **frame)
{
    ENTER;
    PageFrame*  f;
    if (_map_db.Search(address, f)) {
        *frame = f;
        EXIT;
        return ERR_NONE;
    }
    else {
        return ERR_NOT_FOUND;
    }
}

stat_t
Space::Snapshot(addr_t ip, addr_t sp)
{
    ENTER;

    // Dump the mapping DB to a list
    // Note: the list is released in Restore().
    MapList_t*                      list = _map_db.ToList();
    Iterator<MapListElement_t*>&    it = list->GetIterator();

    // Drop writable state of all writable mapped pages and make them
    // copy-on-write to protect them from being overwritten.
    while (it.HasNext()) {
        MapListElement_t*   item = it.Next();
        PageFrame*          frame = item->GetValue();

        DOUT("virt:%.8lX phys:%.8lX state: 0x%X attr: 0x%X\n",
             item->GetKey(), Pg.PhysicalAddress(frame),
             frame->GetState() | frame->GetAccessState(),
             frame->GetAttribute());

        if (frame->IsAccessed(PAGE_STATE_WRITE)) {
            Pg.Unmap(frame, PAGE_PERM_WRITE);
            frame->SetAttribute(PAGE_ATTR_COW | PAGE_ATTR_SNAPSHOT);
            DOUT("Change to COW\n");
        }
        frame->IncrementGeneration();
    }

    ThreadContext *tc = new ThreadContext;
    tc->ip = ip;
    tc->sp = sp;

    DOUT("IP: %.8lX SP: %.8lX DB: %p\n", ip, sp, list);

    MapList_t*      old = _snapshots.Push(list);
    ThreadContext*  old_tc = _thread_context.Push(tc);

    if (old != 0) {
        Int counter = 0;
        it = old->GetIterator();
        while (it.HasNext()) {
            MapListElement_t*   item = it.Next();
            PageFrame*          frame = item->GetValue();

            if (frame->IsSnapshot() &&
                frame->IsAccessed(PAGE_STATE_WRITE) &&
                frame->GetOwner() == _root->Id) {
                // A copy of the page exists.  We can release the original.
                MainPa.Release(frame);
                counter++;
            }
        }
        DOUT("%ld pages are released\n", counter);
        DOUT("Old snapshot released: IP: %.8lX SP: %.8lX AS: %p\n",
             old_tc->ip, old_tc->sp, old);
        delete old;
        delete old_tc;
    }

    EXIT;
    return ERR_NONE;
}

void
Space::DumpMapDB()
{
#ifdef SYS_DEBUG
    MapList_t* dump = _map_db.ToList();
    if (dump == 0) {
        FATAL("out of memory");
    }

    Iterator<MapListElement_t*>& dump_it = dump->GetIterator();
    while (dump_it.HasNext()) {
        MapListElement_t* dump_item = dump_it.Next();
        DOUT("(%.8lX, %p)\n", dump_item->GetKey(), dump_item->GetValue());
    }
#endif
}

void
Space::ReleaseOldFrames(UInt generation, MapList_t* ls)
{
    Iterator<MapListElement_t*>& it = ls->GetIterator();
    while (it.HasNext()) {
        MapListElement_t* item = it.Next();
        PageFrame* frame = item->GetValue();
        if (frame->GetGeneration() < generation) {
            Pg.Unmap(frame, PAGE_PERM_FULL);
            MainPa.Release(frame);
        }
        delete item;
    }
}

stat_t
Space::Restore(UInt generation, addr_t *ip, addr_t *sp)
{
    ENTER;

    if (generation == 0) {
        return ERR_INVALID_ARGUMENTS;
    }

    for (UInt i = 0; i < generation - 1; i++) {
        //XXX: the list and TC are allocated in Snapshot().
        MapList_t* ls = _snapshots.Pop();
        ReleaseOldFrames(generation, ls);
        delete ls;
        delete _thread_context.Pop();
    }

    MapList_t* backup = _snapshots.Pop();
    ThreadContext *tc = _thread_context.Pop();
    if (backup == 0) {
        return ERR_NOT_FOUND;
    }

    //
    // Invalidate the current mappings
    //
    MapList_t* list = _map_db.ToList();
    if (list == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    Iterator<MapListElement_t*>& it = list->GetIterator();

    while (it.HasNext()) {
        MapListElement_t* item = it.Next();
        PageFrame *frame = item->GetValue();
        if (frame->GetGeneration() < generation) {
            Pg.Unmap(frame, PAGE_PERM_FULL);
            MainPa.Release(frame);
        }
        else if (frame->GetGeneration() == generation) {
            frame->SetGeneration(0);
        }
        delete item;
    }
    delete list;

    //
    // Clear the mapping database
    //
    _map_db.Clear();

    //
    // Restore the snapshot
    //

    it = backup->GetIterator();

    while (it.HasNext()) {
        MapListElement_t* item = it.Next();
        DOUT("virt:%.8lX phys:%.8lX state: 0x%X attr: 0x%X\n",
             item->GetKey(), Pg.PhysicalAddress(item->GetValue()),
             item->GetValue()->GetState() | item->GetValue()->GetAccessState(),
             item->GetValue()->GetAttribute());
        if (item->GetKey() == 0 && item->GetValue() == 0) {
            BREAK("nil insertion\n");
        }
        _map_db.Insert(item->GetKey(), item->GetValue());
    }

#ifdef SYS_DEBUG
    it = backup->GetIterator();
    while (it.HasNext()) {
        PageFrame* tmp;
        MapListElement_t* item = it.Next();
        if (_map_db.Search(item->GetKey(), tmp)) {
            if (tmp != item->GetValue()) {
                DOUT("item has different value %p (expected %p)\n",
                     tmp, item->GetValue());
                BREAK("mapping db restoration failed\n");
            }
        }
        else {
            DOUT("item %.8lX has not been registered.\n", item->GetKey());
            DumpMapDB();
            BREAK("mapping db restoration failed\n");
        }
    }
#endif // SYS_DEBUG

    *ip = tc->ip;
    *sp = tc->sp;

    //
    // Push back the data
    //
    _snapshots.Push(backup);
    _thread_context.Push(tc);

    DOUT("Restore IP: %.8lX SP: %.8lX DB: %p\n", *ip, *sp, backup);

    EXIT;
    return ERR_NONE;
}

void
Space::DumpMap()
{
    _map_db.Print();
}

