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
/// @file   Services/Devices/Sata/AhciReceivedFis.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

//$Id: AhciReceivedFis.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_AHCI_RECEIVED_FIS_H
#define ARC_DEVICES_AHCI_RECEIVED_FIS_H

#include <arc/status.h>
#include <arc/types.h>

///
/// Received FIS structure pointed by PxFBU and PxFB
///
class AhciReceivedFis {
public:
    ///
    /// Length of a received FIS structure in the byte, defined by the
    /// AHCI specification.
    ///
    static const UInt   LENGTH = 0x100;

    static const UInt   DSFIS_LENGTH = 0x1C;

    static const UInt   PSFIS_LENGTH = 0x14;

    static const UInt   RFIS_LENGTH = 0x14;

    static const UInt   SDBFIS_LENGTH = 0x8;

    static const UInt   UFIS_LENGTH = 0x40;

    static const addr_t  DSFIS_OFFSET = 0;

    static const addr_t  PSFIS_OFFSET = 0x20;

    static const addr_t  RFIS_OFFSET = 0x40;

    static const addr_t  SDBFIS_OFFSET = 0x58;

    static const addr_t  UFIS_OFFSET = 0x60;

    ///
    /// Pointer to the base address of the region
    ///
    addr_t   base;

    ///
    /// Pointer to the FIS for DMA setup
    ///
    addr_t   dmaSetup;

    ///
    /// Pointer to the FIS for PIO setup
    ///
    addr_t   pioSetup;

    ///
    /// Pointer to the FIS for device-to-host register
    ///
    addr_t   d2hRegister;

    ///
    /// Pointer to the FIS for setting device bits
    ///
    addr_t   deviceBits;

    ///
    /// Pointer to the unknown FIS
    ///
    addr_t   unknown;

    ///
    /// Set the pointers point to inside the given region.  Constructor is not
    /// defined; the new operator is unavailable, because we want to manage
    /// the series of AhciReceivedFis instances as an array.  Each of the
    /// instances may point separate regions.
    ///
    /// @param base     the base address of the region.  It must be 256-byte
    ///                 aligned.  The size of the region is fixed to LENGTH.
    ///
    status_t Initialize(addr_t base);

    ///
    /// Zero-clears the field
    ///
    void Clear();
};

#endif // ARC_DEVICES_AHCI_RECEIVED_FIS_H
