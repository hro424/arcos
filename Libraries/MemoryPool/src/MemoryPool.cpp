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
/// @file   Libraries/MemoryPool/MemoryPool.cpp
/// @brief  Memory pool
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: MemoryPool.cpp 349 2008-05-29 01:54:02Z hro $

#include <MemoryPool.h>
#include <System.h>
#include <String.h>
#include <sys/Config.h>


///
/// A heap frame holds the state of a page in the heap.  A corresponding page
/// is referenced by the index of a frame.
///
struct HeapFrame {
    ///
    /// Unit of page allocation
    ///
    unsigned int count:10;

    ///
    /// Allocation state of the frame.  1 if occupied.
    ///
    unsigned int inUse:1;

    ///
    /// Index to a bin.
    ///
    unsigned int selector:5;
};

#define AQUIRE_LOCK     _mutex.Lock()
#define RELEASE_LOCK    _mutex.Unlock()

/*
HeapBin         Memory::bins[] = {
    {16, PAGE_SIZE, 0, 0, 0, 0},
    {32, PAGE_SIZE, 0, 0, 0, 0},
    {64, PAGE_SIZE, 0, 0, 0, 0},
    {128, PAGE_SIZE, 0, 0, 0, 0},

    {256, PAGE_SIZE, 0, 0, 0, 0},
    {512, PAGE_SIZE, 0, 0, 0, 0},
    {1024, PAGE_SIZE, 0, 0, 0, 0},
    {2048, PAGE_SIZE, 0, 0, 0, 0},

    {0x1000, 0x1000, 0, 0, 0, 0},    // 4k
    {0x2000, 0x2000, 0, 0, 0, 0},    // 8k
    {0x3000, 0x3000, 0, 0, 0, 0},
    {0x4000, 0x4000, 0, 0, 0, 0},    // 16k

    {0x5000, 0x5000, 0, 0, 0, 0},
    {0x6000, 0x6000, 0, 0, 0, 0},
    {0x7000, 0x7000, 0, 0, 0, 0},
    {0x8000, 0x8000, 0, 0, 0, 0},    // 32k

    {0x9000, 0x9000, 0, 0, 0, 0},
    {0xA000, 0xA000, 0, 0, 0, 0},
    {0xB000, 0xB000, 0, 0, 0, 0},
    {0xC000, 0xC000, 0, 0, 0, 0},

    {0xD000, 0xD000, 0, 0, 0, 0},
    {0xE000, 0xE000, 0, 0, 0, 0},
    {0xF000, 0xF000, 0, 0, 0, 0},
    {0x10000, 0x10000, 0, 0, 0, 0},  // 64k
};
*/

#define INIT_BIN(n, s, g, lnk, lcnt, cur, ccnt)   \
    bins[n].size = s;   \
    bins[n].grow = g;   \
    bins[n].link = lnk; \
    bins[n].linkCount = lcnt;   \
    bins[n].cursor = cur;   \
    bins[n].chunkCount = ccnt;

void
MemoryPool::Initialize(word_t start, word_t end)
{
    //ASSERT(start < end);

    // Initialize the 8 first bins to handle sizes 16..4096 bytes and grow by
    // page unit.
    for (int i = 0; i < 8; i++) {
        INIT_BIN(i, 1 << (i + 4), PAGE_SIZE, 0, 0, 0, 0);
    }

    // The last 16 bins are used for larger allocations: from 8KB to 256MB,
    // growing their own size.
    for (int i = 8; i < NBINS; i++) {
        INIT_BIN(i, 1 << (i + 4), 1 << (i + 4), 0, 0, 0, 0);
    }

    start = PAGE_ALIGN(start);
    const ULong size = end - start;

    // How many instances of HeapFrame can we put into a page?
    const UInt framesPerPage = PAGE_SIZE / sizeof(HeapFrame);

    // Allocate the heap frame table at the begging of the given area.
    heapTable = (HeapFrame *)start;

    // Calculate the size of actual heap area.
    // 1. Calculate the number of frames which can fill up the area.
    // 2. Divide that number by (frames-per-page + 1) to exclude the amount of
    //    the memory for the heap frame table. A frame is necessary to each
    //    page.
    heapSize = (size * framesPerPage / (framesPerPage + 1)) & PAGE_MASK;

    heapTableLength = heapSize >> PAGE_BITS;

    heapBase = PAGE_ALIGN(start + heapSize / framesPerPage);

    memset(heapTable, 0, heapTableLength * sizeof(HeapFrame));
}

///
/// Allocates a series of pages.  The region once allocated is never released.
///
/// @param selector     the selector of the bins
/// @param size         the size of the region reserved for the bin at once
///
unsigned long
MemoryPool::AllocatePage(Int selector)
{
    word_t   ptr = 0;
    word_t   cur = 0;
    word_t   size = bins[selector].grow;

    // Look for the specified size of an unused region
retry:
    for (word_t i = cur; i < cur + (size >> PAGE_BITS); i++) {
        if (heapTable[i].inUse == HEAP_INUSE) {
            if (cur < heapTableLength - (size >> PAGE_BITS)) {
                cur = i + 1;
                goto retry;
            }
            else {
                goto exit;
            }
        }
    }

    // Change the state of the region
    for (word_t i = cur; i < cur + (size >> PAGE_BITS); i++) {
        heapTable[i].count = bins[selector].grow / bins[selector].size;
        heapTable[i].inUse = HEAP_INUSE;
        heapTable[i].selector = selector;
    }

    ptr = heapBase + (cur << PAGE_BITS);

exit:
    return ptr;
}

void *
MemoryPool::AllocateAlign(size_t size, word_t alignment)
{
    word_t      ptr;
    Int         index;
    HeapFrame   *frame;

    if (size < alignment) {
        size = alignment;
    }

    // Find the bin fits for the request
    for (index = 0; index < NBINS; index++) {
        if (size <= bins[index].size) {
            break;
        }
    }

    AQUIRE_LOCK;

    // Do we have free blocks? Get one from there...
    if (bins[index].link != 0) {
        ptr = bins[index].link;
        bins[index].link = *(word_t *)bins[index].link;
        bins[index].linkCount--;
    }
    // Get a block from the bin
    else {
        // No available chunk? Try to allocate a memory page for that effect.
        if (bins[index].chunkCount == 0) {
            bins[index].cursor = AllocatePage(index);
            if (bins[index].cursor == 0) {
                RELEASE_LOCK;
                return 0;
            }

            bins[index].chunkCount = bins[index].grow / bins[index].size;
        }

        bins[index].chunkCount--;
        ptr = bins[index].cursor;
        bins[index].cursor += bins[index].size;
    }

    frame = heapTable + ((word_t)(ptr - heapBase) >> PAGE_BITS);
    for (word_t i = 0; i < (bins[index].grow >> PAGE_BITS); i++) {
        frame[i].count--;
    }

    RELEASE_LOCK;

    return (void *)ptr;
}

void *
MemoryPool::Allocate(size_t size)
{
    return AllocateAlign(1, size);
}

void
MemoryPool::Release(void *obj)
{
    HeapFrame   *frame;
    Int         index;
    word_t      ptr = (word_t)obj;

    if (ptr == 0) {
        return;
    }

    //
    // Find out a bin to which the pointer is released
    //
    frame = heapTable + ((ptr - heapBase) >> PAGE_BITS);
    if (frame < heapTable || heapTable + heapTableLength < frame) {
        FATAL("MemoryPool Release");
        return;
    }

    index = frame->selector;

    AQUIRE_LOCK;

    //
    // Update the frame
    //
    for (word_t i = 0; i < (bins[index].grow >> PAGE_BITS); i++) {
        if (frame[0].selector != frame[i].selector) {
            FATAL("MemoryPool Release");
            return;
        }
        frame[i].count++;
    }

    //
    // Update the bin
    //
    *(word_t *)ptr = bins[index].link;
    bins[index].link = ptr;
    bins[index].linkCount++;

    RELEASE_LOCK;
}

size_t
MemoryPool::Length() const
{
    return heapSize;
}

size_t
MemoryPool::Unused() const
{
    size_t total = 0;

    // Count free pages
    for (UInt i = 0; i < heapTableLength; i++) {
        if (heapTable[i].inUse == 0) {
            total += PAGE_SIZE;
        }
    }

    // Count free chunks
    for (Int i = 0; i < NBINS; i++) {
        total += (bins[i].linkCount + bins[i].chunkCount) * bins[i].size;
    }

    return total;
}

