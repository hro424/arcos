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
/// @file   Services/Devices/Sata/AhciCommandTable.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///
//$Id: AhciCommandTable.h 349 2008-05-29 01:54:02Z hro $
#ifndef ARC_DEVICES_AHCI_COMMAND_TABLE_H
#define ARC_DEVICES_AHCI_COMMAND_TABLE_H

#include <arc/status.h>
#include <arc/types.h>
#include "AtaCommandBlock.h"
#include "PhysicalRegionDescriptor.h"

///
/// Structure pointed by a command header
///
class AhciCommandTable {
private:
    static const UInt   PRDT_OFFSET = 0x80;
    static const UInt   FIS_LENGTH = 16;    // length in the double word

public:
    static const UInt   PRDT_LENGTH = 8;
    static const UInt   CFIS_LENGTH = 5;

    ///
    /// The length of the command table.  We limit its length to 256 bytes
    /// (including 8 PRDT entries).
    ///
    static const UInt LENGTH =
            PRDT_OFFSET + sizeof(PhysicalRegionDescriptor) * PRDT_LENGTH;

private:
    ///
    /// Command FIS region
    ///
    UByte _fis[FIS_LENGTH * 4];

    ///
    /// ATAPI command region
    ///
    UByte _atapi[16];

    ///
    /// Reserved region
    ///
    UByte _reserved[48];

    ///
    /// Physical region descriptor table.  The variable is placed in the head
    /// of the array of PRDs.
    ///
    PhysicalRegionDescriptor _prdt[PRDT_LENGTH];

public:
    void *CommandFis();
    void *AtapiCommand();

    ///
    /// Obtains the array of the physical region descriptors
    ///
    PhysicalRegionDescriptor *Prdt();

    status_t BuildCommandFis(AtaCommandBlock *cb);

    void IssueCommand(AtaCommandBlock *cb);

    static size_t PrdtLength(); 

    static size_t CommandFisLength();
};

#endif // ARC_DEVICES_AHCI_COMMAND_TABLE_H
