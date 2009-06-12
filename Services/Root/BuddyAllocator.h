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
/// @file   Services/Root/BuddyAllocator.h
/// @brief  Physical page allocator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

#ifndef ARC_ROOT_BUDDY_ALLOCATOR_H
#define ARC_ROOT_BUDDY_ALLOCATOR_H

#include <Mutex.h>
#include <Types.h>

class PageFrame;
class PageFrameTable;

///
/// A buddy allocator handles free memory areas by listing them into 
/// "chunks lists" called "bins", each managing an increasing order of
/// magnitude.
///
/// When an allocation is performed, a chunk is entirely dedicated
/// to it in the bin which size is equal (or immediatly superior) to the
/// requested size. If the bin is empty, then a chunk from the following bin
/// is split into two "buddies" to satisfy the request.
///
/// Freeing memory follows the inverse process, that is the buddy of the free
/// chunk is checked for availability, and if it is free the two buddies are
/// joined into the following bin.
///
class BuddyAllocator {
public:
    const static unsigned int MAX_ORDER = 13;

private:

    PageFrameTable  *_pft;

    /// Structure referencing all the memory chunks of a magnitude order.
    struct PageBin {
        /// All chunks of same size are linked to each other.
        //list_header_t   link;
        PageFrame       *link;
	/// Number of chunks in the linked list above.
        size_t          count;
	/// Prevent concurrent access to the bin.
        Mutex           mutex;
    };
    
    PageBin         _bins[MAX_ORDER];

    ///
    /// Number of allocated pages.
    ///
    Int             _allocated;

    Int             _reserved;

    ///
    /// Looks for the buddy of the page frame object
    ///
    /// @param frame    the page frame object
    /// @param idx      the index of the page frame object
    /// @param order    the size of buddy in log
    /// @return         the buddy
    ///
    unsigned int FindBuddy(unsigned int idx, unsigned int order);

    ///
    /// Looks for the index of the parent of the page frame object
    ///
    /// @param idx      the index of the page frame
    /// @param order    the size of the region in log
    /// @return         the index of the parent frame
    ///
    unsigned int FindParentIndex(unsigned int idx, unsigned int order);

    ///
    /// Divides the specified region into the half-size two regions.
    ///
    /// @param region	the region to be divided
    /// @return		the last half of the original region
    ///
    stat_t Split(PageFrame *frame, PageFrame **buddy);

    ///
    /// Merges two buddy regions into a single one. The beginning address
    /// of the new frame is returned in the frame parameter.
    ///
    stat_t Merge(PageFrame **frame, PageFrame *buddy);

    void Push(unsigned int order, PageFrame *frame);
    PageFrame *Pop(unsigned int order);
    void Remove(unsigned int order, PageFrame *frame);
    bool IsEmpty(PageBin *bin);

    stat_t AcquireBlock(unsigned int order, PageFrame **frame);
    stat_t ReleaseBlock(PageFrame *frame);

public:
    ///
    /// Allocate a block of the specified order.
    ///
    /// @param order        order of allocation (i.e. a block of size 2^order
    ///                     bytes is requested)
    /// @param frame        address of the requested block will be stored here
    ///
    stat_t Allocate(unsigned int order, PageFrame **frame);

    ///
    /// Releases the specified region.
    ///
    /// @param address      the base address of the region
    ///
    stat_t Release(PageFrame *frame);

    ///
    /// Sets all the bins to neutral values and sets all
    /// the managed area as being free. Also perform a couple
    /// of sanity checks.
    ///
    void Initialize(PageFrameTable *table);

    void PrintMemoryUsage();
};

#endif // ARC_ROOT_BUDDY_ALLOCATOR_H

