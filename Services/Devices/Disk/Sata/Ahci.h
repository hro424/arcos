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
/// @file   Services/Devices/Sata/Ahci.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @date   2007
///

//$Id: Ahci.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_AHCI_H
#define ARC_DEVICES_AHCI_H

#include <arc/debug.h>
#include <arc/status.h>
#include <arc/io.h>
#include <arc/types.h>
#include <arc/util/bitmap.h>
#include "AhciCommandHeader.h"
#include "AhciCommandTable.h"
#include "AhciReceivedFis.h"
#include "MemoryPool.h"

class Ahci {
private:
    ///
    /// The number of ports available
    ///
    UInt _nPorts;

    ///
    /// The number of ports implemented
    ///
    UInt _maxPorts;

    ///
    /// The bitmap for port availability
    ///
    Bitmap_t *_portMap;

    ///
    /// The number of command headers in a command list
    ///
    UInt _nSlots;

    ///
    /// The base virtual address where the controller is mapped.
    ///
    addr_t _base;

    ///
    /// The memory pool for FIS
    ///
    MemoryPool _mempool;

    ///
    /// The pointer to the list of command headers
    ///
    AhciCommandHeader *_commandHeaders;

    ///
    /// The pointer to the list of command tables
    ///
    AhciCommandTable *_commandTables;

    ///
    /// The pointer to the list of received FIS
    ///
    AhciReceivedFis *_receivedFis;

    ///
    /// Investigates the number of active ports and the number of the command
    /// headers per port
    ///
    UInt ProbePorts();

    UInt ProbeSlots();

    ///
    /// Allocates the memory for the structures for ACHI communication
    ///
    status_t InitializeCommandHeaders(UInt ports, UInt slots);

    status_t InitializeReceivedFis(UInt ports, UInt slots);

    ///
    /// Dumps the status of the port
    ///
    void PrintStatus(UInt port);

    void ResetController();

    status_t SoftReset(UInt port);

    status_t HardReset(UInt port);

    status_t SetCommandList(UInt port, addr_t phys);

    status_t SetReceivedFis(UInt port, addr_t phys);

    addr_t Phys(addr_t virt);

public:
    ///
    /// Initializes the object with the specified base address where the AHCI
    /// is mapped.
    ///
    /// @param base     the base virtual address of the AHCI
    ///
    status_t Initialize(addr_t base);

    void Start(UInt port);
    void Stop(UInt port);

    ///
    /// Reads the AHCI control register
    ///
    UInt ReadControl(addr_t reg);

    ///
    /// Writes the data to the AHCI control register
    ///
    void WriteControl(addr_t reg, UInt value);

    ///
    /// Reads the port register
    ///
    UInt ReadPort(UInt port, addr_t reg);

    ///
    /// Writes the data to the port register
    ///
    void WritePort(UInt port, addr_t reg, UInt value);

    ///
    /// Obtains the base address of the AHCI command list
    ///
    AhciCommandHeader *CommandList(UInt port);

    ///
    /// Obtains the specified command header
    ///
    AhciCommandHeader *CommandHeader(UInt port, UInt slot);

    AhciCommandTable *CommandTable(UInt port, UInt slot);

    ///
    /// Obtains the objects that point to the Received FIS structures
    ///
    AhciReceivedFis *ReceivedFis(UInt port);

    ///
    /// Finds a free slot
    ///
    UInt AllocateSlot(UInt port);

    ///
    /// Activate the specified slot
    ///
    void IssueCommand(UInt port, UInt slot);

    Bool IsAvailable(UInt port);

    Bool IsImplemented(UInt port);

    Bool IsRunning(UInt port);

    ///
    /// Shows the information of the AHCI to the display
    ///
    void PrintInfo();
};

#endif // ARC_DEVICES_AHCI_H

