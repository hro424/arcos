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
/// @brief  User task representation
/// @file   Services/P3/Task.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Task.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_PEL2_TASK_H
#define ARC_PEL2_TASK_H

#include <Types.h>
#include <l4/types.h>
#include "Segment.h"

///
/// Representation of the user task
///
struct Task
{
    static const Int    NSEGMENTS = 9;
    static const UInt   USER_INIT = 0;
    static const UInt   USER_READY = 1;

    ///
    /// Entry point of the user program
    ///
    addr_t              entry;

    ///
    /// The main thread of the user task
    ///
    L4_ThreadId_t       main_tid;

    ///
    /// The state of user task
    ///
    UInt                user_state;

    ///
    /// Table of segments
    ///
    Segment*            segment[NSEGMENTS];

    Int                 segment_counter;

    ///
    /// Head segment
    ///
    Segment_Head        head;

    ///
    /// Text segment
    ///
    TextSegment         text;

    ///
    /// Data segment
    ///
    DataSegment         data;

    ///
    /// Persistent memory segment
    ///
    PersistentSegment   pm;

    ///
    /// The segment for sessions
    ///
    SessionSegment      shm;

    ///
    /// The state of the persistent memory
    ///
    Bool                pm_initialized;

    ///
    /// Heap segment
    ///
    HeapSegment         heap;

    ///
    /// Stack segment
    ///
    StackSegment        stack;

    ///
    /// Tail segment (the end of user space)
    ///
    Segment_Tail        tail;

    ///
    /// The default constructor
    ///
    Task();

    ///
    /// Creates a new address space and initializes this object
    ///
    stat_t Initialize(Int state);

    ///
    /// Registers the segment handler
    ///
    void AddSegment(Segment* seg);

    ///
    /// Initializes the text area
    ///
    void InitText(addr_t entry, addr_t base, size_t size);

    ///
    /// Initializes the data area
    ///
    void InitData(addr_t base, size_t size);

    ///
    /// Initializes the bss area
    ///
    void InitBss(addr_t base, size_t size);

    ///
    /// Initializes the persistent memory area
    ///
    void InitPm(addr_t base, size_t size);

    ///
    /// Initializes the heap and the stack areas.  This method is supposed to
    /// be invoked after InitPm.
    ///
    void InitMemory();

    ///
    /// Unmaps the pages apart from the persistent memory area.
    ///
    void Unmap();

    ///
    /// Delete the address space
    ///
    stat_t Delete();

    ///
    /// Start the user task
    ///
    stat_t Start(Int argc, char* argv[], Int state);

    /*
    Thread *CreateThread();
    void DeleteThread();
    */

    Segment *GetSegment(addr_t address);
};

#endif // ARC_PEL2_TASK_H

