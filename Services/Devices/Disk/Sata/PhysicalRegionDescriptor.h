/*
 *
 *  Copyright (C) 2007, Waseda University. All rights reserved.
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
/// @file   Services/Devices/Sata/PhysicalRegionDescriptor.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

// $Id: PhysicalRegionDescriptor.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_SATA_PRD_H
#define ARC_DEVICES_SATA_PRD_H

#include <arc/types.h>

///
/// Physical Region Descriptor Table that contains the scatter/gather list for
/// the data transfer.
///
class PhysicalRegionDescriptor {
private:
    enum {
        INTERRUPT_COMPLETION =      0x80000000U,
        BYTE_COUNT_MASK =           0x003FFFFEU,
    };

    ///
    /// Base address of the data block
    ///
    addr_t   _base;

    ///
    /// Upper 32-bit of the base address of the data block
    ///
    addr_t   _baseUpper;

    ///
    /// Reserved region
    ///
    addr_t   _reserved;

    ///
    /// The length of the data block.  It also contains the interrupt
    /// completion bit at MSB.
    ///
    size_t    _count;

public:
    ///
    /// Registers the data block to the descriptor
    ///
    /// @param addr     the base address of the data block
    /// @param length   the length of the data block in byte
    ///
    void SetDataBlock(addr_t addr, size_t length);

    void UnsetDataBlock();

    ///
    /// Obtains the base address of the data block currently set
    ///
    addr_t BaseAddress();

    ///
    /// Obtains the length of the data block currently set in byte
    ///
    size_t Length();

    ///
    /// Make the device assert an interrupt upon the completion of transfer
    ///
    void SetInterrupt();

    ///
    /// Make the device be silent upon the completion of transfer
    ///
    void ClearInterrupt();

    ///
    /// Test if the interrupt completion is set.
    ///
    bool IsInterrupt();
};

#endif  // ARC_DEVICES_SATA_PRD_H

