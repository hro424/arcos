/*
 *
 *  Copyright (C) 2008, Waseda University.
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
/// @brief  Memory manager API
/// @file   Include/arc/MemoryManager.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: MemoryManager.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_MEMORY_MANAGER_H
#define ARC_MEMORY_MANAGER_H

#include <Types.h>
#include <l4/types.h>

class MemoryManager
{
private:
    stat_t Map(addr_t dest, L4_ThreadId_t did, UInt rwx,
               addr_t src, L4_ThreadId_t sid);

public:
    ///
    /// Reserves an anonymous page for the base.  The permission of the
    /// reserved page is read-write-only.
    ///
    stat_t Reserve(addr_t base);

    ///
    /// Reserves an anonymous page for the base and the peer.  The permission
    /// of the reserve page is read-write-only.
    ///
    stat_t Reserve(addr_t base, L4_ThreadId_t peer, UInt rwx);

    ///
    /// An anonymous page is mapped to the destination.
    ///
    stat_t Map(addr_t dest, UInt rwx);

    ///
    /// The specified page is mapped to the destination.  If the source TID is
    /// the nil thread, the source address represents a physical address.  If
    /// the source TID is the any thread, the source address is ignored and an
    /// anonymous page is mapped to the destination.
    /// The behavior changes depending on the sid.
    /// sid = nilthread:    the source address represents a physical address.
    /// sid = anythread:    the source address is ignored and an anonymous page
    ///                     is mapped to the destination.
    /// sid = self:         the source address is ignored and a reserved page
    ///                     is mapped to the destination.
    ///
    /// @param dest     the destination
    /// @param rwx      the requesting permission
    /// @param src      the source address.
    /// @param sid      the source address space.
    ///
    stat_t Map(addr_t dest, UInt rwx, addr_t src, L4_ThreadId_t sid);

    ///
    /// Unmaps and releases the specified page.
    ///
    stat_t Release(addr_t base);

    stat_t Expand(size_t count);

    Bool IsMapped(addr_t address, UInt rwx);

    ///
    /// Obtains the physical address of the virtual address
    ///
    /// @param virt         the virtual address
    /// @return ~0 if the given virtual address is invalid.
    ///
    addr_t Phys(addr_t virt);
};

extern MemoryManager    Pager;

#endif // ARC_MEMORY_MANAGER_H

