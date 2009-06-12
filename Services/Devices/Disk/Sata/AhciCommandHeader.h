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
/// @file   Services/Devices/Sata/AhciCommandHeader.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: AhciCommandHeader.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_AHCI_COMMAND_HEADER_H
#define ARC_DEVICES_AHCI_COMMAND_HEADER_H

#include <arc/types.h>
#include "AhciCommandTable.h"

///
/// An entry of a command list structure
///
class AhciCommandHeader {
private:
    enum {
        PRDTL_MASK =        0xFFFF0000U,
        PMP_MASK =          0x0000F000U,
        CLEAR_STATUS =      0x00000400U,
        BIST =              0x00000200U,
        RESET =             0x00000100U,
        PREFETCHABLE =      0x00000080U,
        DEVICE_WRITE =      0x00000040U,
        ATAPI_COMMAND =     0x00000020U,
        CFL_MASK =          0x0000001FU,
    };

    UInt    _head;
    UInt    _count;
    UInt    _base;
    UInt    _baseUpper;
    UInt    _reserved[4];

public:
    ///
    /// Sets the number of the physical region descriptor table entries
    ///
    void SetPrdtl(size_t length);

    ///
    /// Gets the length of the physical region descriptor table
    ///
    UInt Prdtl();

    ///
    /// Sets the port multiplier port
    ///
    void SetPmp(UInt port);

    ///
    /// Gets the port multiplier port
    ///
    UInt Pmp();

    ///
    /// Makes the controller clear PxTFD.STS.BSY and PxCI.CI after
    /// transmitting the FIS and receiving R_OK.
    ///
    void SetClearStatus();
    void ClearClearStatus();
    Bool IsClearStatus();

    void SetBist();
    void ClearBist();
    Bool IsBist();

    void SetReset();
    void ClearReset();
    Bool IsReset();

    void SetPrefetchable();
    void ClearPrefetchable();
    Bool IsPrefetchable();

    void SetDeviceWrite();
    void ClearDeviceWrite();
    Bool IsDeviceWrite();

    void SetAtapiCommand();
    void ClearAtapiCommand();
    Bool IsAtapiCommand();

    ///
    /// Stores the length of the command FIS
    ///
    void SetCommandFisLength(size_t length);

    ///
    /// Gets the length of the command FIS
    ///
    UInt CommandFisLength();

    ///
    /// Stores the length of the physical region descriptor
    ///
    void SetByteCount(UInt c);

    ///
    /// Gets the length of the physical region descriptor
    ///
    UInt ByteCount();

    ///
    /// Stores the physical address of the command table
    ///
    void SetCommandTable(addr_t addr);

    ///
    /// Gets the length of the physical region descriptor
    ///
    AhciCommandTable *CommandTable();
};

#endif // ARC_DEVICES_AHCI_COMMAND_HEADER_H

