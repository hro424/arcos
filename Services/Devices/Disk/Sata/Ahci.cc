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
/// @file   Services/Devices/Sata/Ahci.cc
/// @brief  AHCI driver code
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @date   2007
///

//$Id: Ahci.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/memory.h>
#include <arc/system.h>
#include <arc/io.h>
#include <arc/console.h>
#include <arc/status.h>
#include <arc/string.h>
#include <arc/types.h>
#include <arc/util/bitmap.h>
#include "Ahci.h"
#include "AhciCommandHeader.h"
#include "AhciCommandTable.h"
#include "AhciReceivedFis.h"
#include "MemoryPool.h"
#include "Runtime.h"

#include <l4/types.h>
#include <l4/ipc.h>

static const addr_t PORT_BASE[4] = {
    ABAR_P0PCR, ABAR_P1PCR, ABAR_P2PCR, ABAR_P3PCR
};

UInt
Ahci::ReadControl(addr_t reg) {
    return *(UInt *)(_base + reg);
}

void
Ahci::WriteControl(addr_t reg, UInt value) {
    *(volatile UInt *)(_base + reg) = value;
}

UInt
Ahci::ReadPort(UInt port, addr_t reg)
{
    return ReadControl(PORT_BASE[port] + reg);
}

void
Ahci::WritePort(UInt port, addr_t reg, UInt value)
{
    WriteControl(PORT_BASE[port] + reg, value);
}


Bool
Ahci::IsImplemented(UInt port)
{
    UInt data;

    data = ReadControl(ABAR_PI);
    return (((data >> port) & 1) == 1U);
}

Bool
Ahci::IsRunning(UInt port)
{
    UInt data;

    if (port < _maxPorts) {
        data = ReadPort(port, ABAR_PXCMD);

        return ((data & ABAR_PXCMD_ST) == ABAR_PXCMD_ST ||
                (data & ABAR_PXCMD_CR) == ABAR_PXCMD_CR ||
                (data & ABAR_PXCMD_FRE) == ABAR_PXCMD_FRE ||
                (data & ABAR_PXCMD_FR) == ABAR_PXCMD_FR);
    }
    return FALSE;
}

Bool
Ahci::IsAvailable(UInt port)
{
    if (port < _maxPorts) {
        //return _portMap.Test(port);
        return (BITMAP_GET(port, _portMap) == 1);
    }
    return FALSE;
}

UInt
Ahci::ProbeSlots()
{
    UInt    data;
    UInt    slots;

    ENTER;

    //
    // Investigate on the capabilities of the controller
    //
    data = ReadControl(ABAR_CAP);

    //
    // Determine the number of the command headers in the command list.
    // It's hardwired to 0x1F to indicate support for 32 slots.
    //
    slots = ((data & ABAR_CAP_NCS) >> 8) + 1;
    if (slots != ABAR_CAP_NCS_DEFAULT) {
        ConsoleOut(WARN, "Different number of command slots: %lu\n", slots);
        slots = ABAR_CAP_NCS_DEFAULT;
    }

    EXIT;
    return slots;
}

UInt
Ahci::ProbePorts()
{
    UInt    ports;
    UInt  data;

    ENTER;

    //
    // Determine which ports are implemented
    //
    ports = 0;
    data = ReadControl(ABAR_PI);
    for (Int i = 0; i < 32; i++) {
        if (((data >> i) & 1) == 1U) {
            ports++;
        }
    }

    EXIT;
    return ports;
}

//
// For each of 4 ports (you can change):
// 1024 bytes for 32 command headers (32 bytes each)
// 256 bytes for a received FIS
// 8192 bytes for 32 command tables (256 bytes each)
//
status_t
Ahci::InitializeCommandHeaders(UInt ports, UInt slots)
{
    addr_t      phys;
    UInt        size;
    status_t    stat;

    ENTER;

    size = ((sizeof(AhciCommandHeader) + AhciCommandTable::LENGTH) * slots +
            AhciReceivedFis::LENGTH) * ports;

    // Construct a memory pool for I/O mapping
    stat = _mempool.Initialize(size);
    if (stat != ERR_NONE) {
        return stat;
    }

    // Allocate command list. It must be aligned to 0x80.
    size = sizeof(AhciCommandHeader) * slots * ports;
    stat = _mempool.AllocateAlign(size, 0x80, (void **)&_commandHeaders);
    memset(_commandHeaders, 0, size);

    // Allocate command table in the fixed length (8 PRDT entries)
    size = AhciCommandTable::LENGTH * slots * ports;
    stat = _mempool.AllocateAlign(size, 0x80, (void **)&_commandTables);
    memset(_commandTables, 0, size);

    phys = _mempool.Phys((addr_t)_commandTables);
    for (UInt i = 0; i < slots * ports; i++) {
        _commandHeaders[i].SetCommandTable(phys + AhciCommandTable::LENGTH * i);
        _commandHeaders[i].SetCommandFisLength(AhciCommandTable::CFIS_LENGTH);
    }

    for (UInt i = 0; i < ports; i++) {
        phys = _mempool.Phys((addr_t)_commandHeaders) +
            sizeof(AhciCommandHeader) * slots * i & ABAR_PXCLB_MASK;
        DOUT("Set command list @ %.8lX (phys)\n", phys);
        SetCommandList(i, phys);
    }

    EXIT;
    return ERR_NONE;
}

status_t
Ahci::InitializeReceivedFis(UInt ports, UInt slots)
{
    addr_t fis;
    addr_t phys;

    // Allocate Received FIS management structures
    _receivedFis = (AhciReceivedFis *)malloc(sizeof(AhciReceivedFis) * ports);
    if (_receivedFis == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    DOUT("received FIS @ %p\n", _receivedFis);
    DOUT("%.8lX%.8lX\n", _portMap[0], _portMap[1]);

    _mempool.AllocateAlign(AhciReceivedFis::LENGTH * ports, 0x100UL,
                           (void **)&fis);

    DOUT("%.8lX%.8lX\n", _portMap[0], _portMap[1]);

    memset((void *)fis, 0, AhciReceivedFis::LENGTH * ports);

    DOUT("Allocate received FIS @ v:0x%.8lX p:0x%.8lX\n",
         fis, _mempool.Phys(fis));
    DOUT("%.8lX%.8lX\n", _portMap[0], _portMap[1]);

    for (UInt i = 0; i < ports; i++) {
        _receivedFis[i].Initialize(fis + AhciReceivedFis::LENGTH * i);
        phys = _mempool.Phys(_receivedFis[i].base) & ABAR_PXFB_MASK;
        SetReceivedFis(i, phys);
    }

    DOUT("%.8lX%.8lX\n", _portMap[0], _portMap[1]);

    return ERR_NONE;
}

void
Ahci::ResetController()
{
    UInt    data;
    ENTER;

    // Enable AHCI
    WriteControl(ABAR_GHC, ABAR_GHC_AE);
    data = ReadControl(ABAR_GHC);

    // Reset HBA
    WriteControl(ABAR_GHC, ABAR_GHC_AE | ABAR_GHC_HR);
    data = ReadControl(ABAR_GHC);

    // Enable AHCI
    WriteControl(ABAR_GHC, ABAR_GHC_AE);
    data = ReadControl(ABAR_GHC);

    //
    // Sleep at least 500 ms for the reset
    //
    L4_Sleep(L4_TimePeriod(500000));

    EXIT;
}

status_t
Ahci::SoftReset(UInt port)
{
    UInt data;
    ENTER;

    //
    // Clear PxCMD.ST and PxCMD.FR. Turn AHCI into the idle state.
    //
    data = ReadPort(port, ABAR_PXCMD);
    data &= ~ABAR_PXCMD_ST;
    data &= ~ABAR_PXCMD_FR;
    WritePort(port, ABAR_PXCMD, data);

    //
    // Wait at least 500 ms for PxCMD.CR (Controller Running) and
    // PxCMD.FR (FIS Receive Running) to be 0
    //
    L4_Sleep(L4_TimePeriod(500000UL));

    for (int i = 0; i < 100; i++) {
        data = ReadPort(port, ABAR_PXCMD);
        if ((data & ABAR_PXCMD_CR) == 0 &&
            (data & ABAR_PXCMD_FR) == 0) {
            break;
        }
    }

    if ((data & ABAR_PXCMD_CR) == ABAR_PXCMD_CR ||
        (data & ABAR_PXCMD_FR) == ABAR_PXCMD_FR) {
        // They are still busy.
        return ERR_TIMEOUT;
    }

    EXIT;
    return ERR_NONE;
}


status_t
Ahci::HardReset(UInt port)
{
    UInt    data;
    ENTER;

    // PxCMD.ST must be 0.
    data = ReadPort(port, ABAR_PXCMD);
    if ((data & ABAR_PXCMD_ST) == ABAR_PXCMD_ST) {
        return ERR_AGAIN;
    }

    // Hard reset to re-initialize the communication
    data = ReadPort(port, ABAR_PXSCTL);
    data |= ABAR_PXSCTL_DET_COMM;
    WritePort(port, ABAR_PXSCTL, data);
    L4_Sleep(L4_TimePeriod(2000));

    // PxSCTL.DET is still 1 after hard reset. So reset the field to 0.
    data = ReadPort(port, ABAR_PXSCTL);
    data &= ~ABAR_PXSCTL_DET_MASK;
    WritePort(port, ABAR_PXSCTL, data);
    L4_Sleep(L4_TimePeriod(500000));

    do {
        data = ReadPort(port, ABAR_PXSSTS);
    } while ((data & ABAR_PXSSTS_DET_MASK) != ABAR_PXSSTS_DET_DEV);

    // Clear the error state
    data = ReadPort(port, ABAR_PXSERR);
    WritePort(port, ABAR_PXSERR, data);
    data = ReadPort(port, ABAR_PXSERR);

    EXIT;
    return ERR_NONE;
}

// Set the pointer to a command list (physical address)
status_t
Ahci::SetCommandList(UInt port, addr_t phys)
{
    UInt    data;

    WritePort(port, ABAR_PXCLB, phys);
    WritePort(port, ABAR_PXCLBU, 0);
    data = ReadPort(port, ABAR_PXCLB);

    return ERR_NONE;
}

// Set the pointer to a received FIS (physical address)
status_t
Ahci::SetReceivedFis(UInt port, addr_t phys)
{
    UInt    data;

    data = ReadPort(port, ABAR_PXCMD);
    data &= ~ABAR_PXCMD_FRE;
    do {
        data = ReadPort(port, ABAR_PXCMD);
    } while (ABAR_PXCMD_FR == (data & ABAR_PXCMD_FR));

    WritePort(port, ABAR_PXFB, phys);
    WritePort(port, ABAR_PXFBU, 0);

    //
    // Allow to receive FIS from the device
    //
    data = ReadPort(port, ABAR_PXCMD);
    data |= ABAR_PXCMD_FRE;
    WritePort(port, ABAR_PXCMD, data);

    do {
        data = ReadPort(port, ABAR_PXCMD);
    } while (ABAR_PXCMD_FR != (data & ABAR_PXCMD_FR));

    return ERR_NONE;
}


//-----------------------------------------------------------------------------
//      Public Member Methods
//-----------------------------------------------------------------------------

status_t
Ahci::Initialize(addr_t base)
{
    UInt        data;
    status_t    stat;
    
    ENTER;

    _base = base;
    _portMap = (Bitmap_t *)malloc(BITMAP_ALLOCSIZE(32));
    if (_portMap == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    BITMAP_INIT(_portMap, 32);
    _nPorts = 0;

    //
    // AHCI Minimal Initialization Procedure
    // Reference:
    // [1] Amber Huffman, Serial ATA Advanced Host Controller Interface (AHCI)
    //     Rev. 1.1, Intel corporation, pp 81-82.
    //

    ResetController();

    _nSlots = ProbeSlots();
    _maxPorts = ProbePorts();

    for (UInt port = 0; port < _maxPorts; port++) {
        //
        // Clear the error status
        //
        data = ReadPort(port, ABAR_PXSERR);
        WritePort(port, ABAR_PXSERR, data);
        data = ReadPort(port, ABAR_PXSERR);

        //
        // Check the port is in an idle state.
        //
        if (IsRunning(port)) {
            if (SoftReset(port) != ERR_NONE) {
                ConsoleOut(WARN, "Ahci port %lu is busy. Skip.\n", port);
                continue;
            }
        }

        //
        // Check if the device is attached
        //
        data = ReadPort(port, ABAR_PXSSTS);
        if ((data & ABAR_PXSSTS_DET_DEV) != 1) {
            ConsoleOut(WARN, "Ahci port %lu is unattached. Skip.\n", port);
            continue;
        }

        BITMAP_SET(port, _portMap);
        _nPorts++;
    }

    DOUT("ports: %lu/%lu slots: %lu\n", _nPorts, _maxPorts, _nSlots);

    // Allocate memory pool for the data structures for AHCI
    stat = InitializeCommandHeaders(_nPorts, _nSlots);
    if (stat != ERR_NONE) {
        return stat;
    }

    stat = InitializeReceivedFis(_nPorts, _nSlots);
    if (stat != ERR_NONE) {
        return stat;
    }

    for (UInt port = 0; port < _nPorts; port++) {
        //
        // Clear the error registers (write clear)
        //
        data = ReadPort(port, ABAR_PXSERR);
        WritePort(port, ABAR_PXSERR, data);

        //
        // Wait for the device ready
        //
        do {
            data = ReadPort(port, ABAR_PXTFD);
        } while ((data & ABAR_PXTFD_STS_BSY) != 0 ||
                 (data & ABAR_PXTFD_STS_DRQ) != 0 ||
                 (data & ABAR_PXTFD_STS_ERR) != 0);

        //
        // Set up the interrupt status registers (write clear)
        //
        //WritePort(port, ABAR_PXIS, ~0UL);

        //
        // Set up the interrupt mask (mask all)
        //
        WritePort(port, ABAR_PXIE, 0);

        Start(port);
    }

    //
    // Enable the interrupt from the controller
    //
    data = ReadControl(ABAR_GHC);
    WriteControl(ABAR_GHC, data | ABAR_GHC_IE);

    // Write clear the interrupt status register
    WriteControl(ABAR_IS, ~0UL);

    EXIT;
    return ERR_NONE;
}

void
Ahci::Start(UInt port)
{
    UInt    data;

    ENTER;

    data = ReadPort(port, ABAR_PXCMD);
    data |= ABAR_PXCMD_ST;
    WritePort(port, ABAR_PXCMD, data);

    WritePort(port, ABAR_PXIE, ~0UL);
    data = ReadPort(port, ABAR_PXIE);

    WritePort(port, ABAR_PXIS, ~0UL);
    data = ReadPort(port, ABAR_PXIS);
    WriteControl(ABAR_IS, ~0UL);
    data = ReadControl(ABAR_IS);

    EXIT;
}

void
Ahci::Stop(UInt port)
{
    UInt    data;

    ENTER;

    WritePort(port, ABAR_PXIE, 0);
    data = ReadPort(port, ABAR_PXIE);

    data = ReadPort(port, ABAR_PXCMD);
    data &= ~ABAR_PXCMD_ST;
    WritePort(port, ABAR_PXCMD, data);

    WritePort(port, ABAR_PXIS, ~0UL);
    data = ReadPort(port, ABAR_PXIS);
    WriteControl(ABAR_IS, ~0UL);
    data = ReadControl(ABAR_IS);

    EXIT;
}


AhciCommandHeader *
Ahci::CommandList(UInt port)
{
    AhciCommandHeader *obj = (AhciCommandHeader *)0;
    if (port < _nPorts) {
        obj = &_commandHeaders[_nSlots * port];
    }
    return obj;
}

AhciCommandHeader *
Ahci::CommandHeader(UInt port, UInt slot)
{
    AhciCommandHeader *obj = (AhciCommandHeader *)0;
    if (port < _nPorts && slot < _nSlots) {
        obj = &_commandHeaders[_nSlots * port + slot];
    }
    return obj;
}

AhciCommandTable *
Ahci::CommandTable(UInt port, UInt slot)
{
    AhciCommandTable *obj = (AhciCommandTable *)0;
    if (port < _nPorts && slot < _nSlots) {
        obj = &_commandTables[_nSlots * port + slot];
    }
    return obj;
}

AhciReceivedFis *
Ahci::ReceivedFis(UInt port)
{
    AhciReceivedFis *obj = (AhciReceivedFis *)0;

    if (port < _nPorts) {
        obj = &_receivedFis[port];
    }
    return obj;
}

UInt
Ahci::AllocateSlot(UInt port)
{
    UInt  data;
    UInt    slot;

    data = ReadPort(port, ABAR_PXCI);
    for (slot = 0; slot < 32; slot++) {
        if (((data >> slot) & 1) == 0) {
            break;
        }
    }

    return slot;
}


void
Ahci::IssueCommand(UInt port, UInt slot)
{
    WritePort(port, ABAR_PXCI, 1 << slot);
}


void
Ahci::PrintStatus(UInt port)
{
    UInt    data;

    data = ReadPort(port, ABAR_PXTFD);
    printf("Port %lu stat: %s%s%s%s (%.4lX)\n",
           port,
           (data & ABAR_PXTFD_ERR) ? "err " : "",
           (data & ABAR_PXTFD_STS_BSY) ? "sbsy " : "",
           (data & ABAR_PXTFD_STS_DRQ) ? "sdrq " : "",
           (data & ABAR_PXTFD_STS_ERR) ? "serr " : "",
           data);
    data = ReadPort(port, ABAR_PXSERR);
    printf("\terr: %.8lX\n", data);
    data = ReadPort(port, ABAR_PXIS);
    printf("\tint: %.8lX\n", data);
}

void
Ahci::PrintInfo()
{
    UInt reg;

    reg = ReadControl(ABAR_VS);
    printf("AHCI Version %lX.%lX\n",
           (reg >> 16), (reg & ABAR_VS_MNR) >> 4);

    reg = ReadControl(ABAR_CAP);
    printf("cap: %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
           ((reg & ABAR_CAP_S64A) ? "s64a " : ""),
           ((reg & ABAR_CAP_SCQA) ? "scqa " : ""),
           ((reg & ABAR_CAP_SSNTF) ? "ssntf " : ""),
           ((reg & ABAR_CAP_SIS) ? "sis " : ""),
           ((reg & ABAR_CAP_SSS) ? "sss " : ""),

           ((reg & ABAR_CAP_SALP) ? "salp " : ""),
           ((reg & ABAR_CAP_SCLO) ? "sclo " : ""),
           ((reg & ABAR_CAP_SNZO) ? "snzo " : ""),
           ((reg & ABAR_CAP_SPSA) ? "spsa " : ""),
           ((reg & ABAR_CAP_PMS) ? "pms " : ""),

           ((reg & ABAR_CAP_PMFS) ? "pmfs " : ""),
           ((reg & ABAR_CAP_PMD) ? "pmd " : ""),
           ((reg & ABAR_CAP_SSC) ? "ssc " : ""),
           ((reg & ABAR_CAP_PSC) ? "psc " : ""));

    reg = ReadControl(ABAR_PI);
    printf("port: %s%s%s%s\n",
           ((reg & ABAR_PI_PI0) ? "0 " : ""),
           ((reg & ABAR_PI_PI1) ? "1 " : ""),
           ((reg & ABAR_PI_PI2) ? "2 " : ""),
           ((reg & ABAR_PI_PI3) ? "3 " : ""));
}

