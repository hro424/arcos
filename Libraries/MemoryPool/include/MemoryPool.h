/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @brief  Fixed-size heap
/// @file   Include/arc/MemoryPool.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: MemoryPool.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_MEMORY_POOL_H
#define ARC_MEMORY_POOL_H

#include <Mutex.h>
#include <Types.h>

struct HeapFrame;

struct HeapBin {
    ///
    /// The size of the bin
    ///
    word_t size;

    ///
    /// The size that the bin grows
    ///
    word_t grow;

    ///
    /// The pointer to the first free block of the free list
    ///
    word_t link;

    ///
    /// The number of free blocks in the free list
    ///
    word_t linkCount;

    ///
    /// The pointer to the next free block of the region
    ///
    word_t cursor;

    ///
    /// The number of free blocks in the page
    ///
    word_t chunkCount;
};


///
/// Handles heap-type memory.
///
class MemoryPool {
private:
    static const UInt   HEAP_FREE = 0;
    static const UInt   HEAP_INUSE = 1;
    static const Int    NBINS = 24;

    ///
    /// A bin represents a collection of frames.
    ///
    HeapBin     bins[NBINS];

    ///
    /// A frame represents a fraction of the memory pool.
    ///
    HeapFrame*  heapTable;

    ///
    /// Number of HeapFrames in the area pointed by heapTable.
    ///
    word_t      heapTableLength;

    word_t      heapBase;
    word_t      heapSize;
    Mutex       _mutex;

    word_t AllocatePage(Int selector);

public:
    void Initialize(word_t start, word_t end);
    void *AllocateAlign(size_t size, word_t align);
    void *Allocate(size_t size);
    void Release(void *ptr);

    ///
    /// The capacity of the memory pool
    ///
    size_t Length() const;

    ///
    /// The size of used memory
    ///
    size_t Unused() const;
};

#endif // ARC_MEMORY_POOL_H

