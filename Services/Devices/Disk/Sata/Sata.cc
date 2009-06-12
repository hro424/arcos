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
/// @file   Services/Devices/Sata/Sata.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Sata.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/io.h>
#include <arc/ipc.h>
#include <arc/memory.h>
#include <arc/protocol.h>
#include <arc/system.h>
#include <arc/status.h>
#include <arc/string.h>
#include <arc/types.h>
#include <arc/util/bitmap.h>
#include "Ahci.h"
#include "AhciCommandHeader.h"
#include "AhciCommandTable.h"
#include "Ata.h"
#include "AtaCommandBlock.h"
#include "Runtime.h"
#include "Sata.h"

#include <l4/kdebug.h>
#include <l4/ipc.h>

#include <stdio.h>

#define BUFFER_SIZE     4096
static char infoBuffer[BUFFER_SIZE] __attribute__ ((aligned (4096)));

static Sata             _driver;
static L4_ThreadId_t    _main;


status_t
Sata::SetupAhci(void)
{
    L4_Msg_t        msg;
    addr_t          dest;
    addr_t          phys;
    L4_Word_t       reg[2];
    status_t        err;

    ENTER;

    dest = palloc(1);
    //TODO: remap

    //
    // Allocate the page where AHCI will be mapped. (get a physical address)
    //
    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));

    reg[0] = dest;
    reg[1] = 1;
    L4_Put(&msg, MSG_PEL_IOMAP | L4_ReadWriteOnly,
           2, (L4_Word_t *)reg, 0, (void *)0);
    err = IpcCall(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        pfree(dest, 1);
        return err;
    }

    L4_Accept(L4_MapGrantItems(L4_Nilpage));

    // Get physical address
    phys = (addr_t)L4_Get(&msg, 0);
    // Get virtual address
    //L4_Get(&msg, 0, &item);
    //dest = L4_SndBase(item);

    DOUT("I/O mapped area: (phys) %.8lX\n", phys);

    //
    // Map AHCI to the physical address
    //
    PCI_Write32(ICH_SATA, PCI_ABAR, phys);

    if (_ahci.Initialize(dest) != ERR_NONE) {
        //TODO: Unmap
        pfree(dest, 1);
        return ERR_UNKNOWN;
    }

    EXIT;
    return ERR_NONE;
}


status_t
Sata::Initialize()
{
    UInt        reg;
    status_t    stat;

    ENTER;

    reg = PCI_Read16(ICH_SATA, PCI_PCICMD);

    if ((reg & PCI_PCICMD_MSE) != PCI_PCICMD_MSE) {
        return ERR_NOT_FOUND;
    }

    stat = SetupAhci();
    if (stat != ERR_NONE) {
        ConsoleOut(ERROR, "AHCI initialization failed.\n");
        return stat;
    }
    
    _ahci.PrintInfo();

    // setup interrupt
    SetInterrupt(5);

    EXIT;
    return ERR_NONE;
}

//XXX: Workaround
void
Sata::Init()
{
    AtaCommandBlock cb;

    for (UInt i = 0; i < 2; i++) {
        if (_ahci.IsAvailable(i)) {
            //_ahci.Start(i);

            //
            // Get the identity of the drives
            // 
            cb.Initialize();
            cb.command = ATA_CMD_IDENTIFY;
            memset(infoBuffer, 0, BUFFER_SIZE);

            DataIn(i, &cb, infoBuffer, sizeof(ATA_DeviceInfo_t));
            PrintDeviceInfo((unsigned short *)infoBuffer);

            //_ahci.Stop(i);
        }
    }

}

void
Sata::HandleUnknownFis(UInt port)
{
    ENTER;
    EXIT;
}

void
Sata::HandleSetDeviceBits(UInt port)
{
    ENTER;
    EXIT;
}

//
// Receive PIO Setup FIS
//
void
Sata::HandlePioSetupFis(UInt port)
{
    AhciReceivedFis *fis;
    volatile UByte *ptr;
    ENTER;

    // Read a received FIS
    fis = _ahci.ReceivedFis(port);
    ptr = (volatile UByte *)fis->pioSetup;
    if (ptr[0] != FIS_TYPE_PIO_SETUP) {
        ConsoleOut(WARN, "FIS type mismatch: PIO Setup: %X %X\n",
                      ptr[0], FIS_TYPE_PIO_SETUP);
        DumpReceivedFis(fis);
        //return ERR_AGAIN;
    }

    EXIT;
}

void
Sata::HandleDmaSetupFis(UInt port)
{
    ENTER;
    EXIT;
}

void
Sata::HandleDeviceToHostRegisterFis(UInt port)
{
    ENTER;

    EXIT;
}

void
Sata::HandleDataFis(UInt port)
{
    ENTER;

    //
    // Receive Data FIS
    //
    _ahci.WritePort(port, ABAR_PXIS, _ahci.ReadPort(port, ABAR_PXIS));
    _ahci.WriteControl(ABAR_IS, 2U);


    EXIT;
}

status_t
Sata::HandleInterrupts(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    UInt istat;
    ENTER;

    /*
    if (!L4_ThreadEqual(tid, L4_GlobalId(num, 1))) {
        return Arc_ReturnError(ERR_NONE, msg);
    }
    */

    // Identify the port pending
    istat = _ahci.ReadControl(ABAR_IS);
    if (istat == 0) {
        DOUT("Something's wrong.\n");
        /*
        // First clear the event from the PxIS register, then clear the
        // interrupt event from the IS register.
        for (int i = 0; i < 2; i++) {
            _ahci.WritePort(i, ABAR_PXIS, ~0UL);
        }
        _ahci.WriteControl(ABAR_IS, ~0UL);
        L4_Sleep(L4_TimePeriod(1000000UL));

        do {
            stat = _ahci.ReadControl(ABAR_IS);
        } while (stat == ~0UL);
        */
        /*
        _ahci.WriteControl(ABAR_IS, stat);
        stat = _ahci.ReadControl(ABAR_IS);
        for (int i = 0; i < 4; i++) {
            AhciReceivedFis *fis;
            DumpStatus(i);
            fis = _ahci.ReceivedFis(i);
            DumpReceivedFis(fis);
        }
        */
        DumpStatus(0);
        L4_Put(msg, 0, 0, (L4_Word_t *)0, 0, (void *)0);
        L4_Load(msg);
        L4_Send(_main);
        return IpcReturnError(msg, ERR_NONE);
    }

    for (UInt port = 0; port < 4; port++) {
        if (((istat >> port) & 1) == 1) {
            UInt itype;

            //DumpStatus(port);

            // Identify the type of the interrupt
            itype = _ahci.ReadPort(port, ABAR_PXIS);

            if ((itype & ABAR_PXIS_UFS) == ABAR_PXIS_UFS) {
                HandleUnknownFis(port);
            }

            if ((itype & ABAR_PXIS_SDBS) == ABAR_PXIS_SDBS) {
                HandleSetDeviceBits(port);
            }

            if ((itype & ABAR_PXIS_DSS) == ABAR_PXIS_DSS) {
                HandleDataFis(port);
            }

            if ((itype & ABAR_PXIS_PSS) == ABAR_PXIS_PSS) {
                HandlePioSetupFis(port);
            }

            if ((itype & ABAR_PXIS_DHRS) == ABAR_PXIS_DHRS) {
                HandleDeviceToHostRegisterFis(port);
            }

            //
            // Notification IPC (async)
            //
            L4_Put(msg, 0, 1, (L4_Word_t *)&port, 0, (void *)0);
            L4_Load(msg);
            L4_Send(_main);

            _ahci.WritePort(port, ABAR_PXIS, ~0UL);
        }
    }
    _ahci.WriteControl(ABAR_IS, ~0UL);

    EXIT;
    return IpcReturnError(msg, ERR_NONE);
}

//
//  SATA Operations
//

status_t
Sata::Open(UInt port)
{
    // Check if the specified port is available.

    //
    // Open the port
    //

    //
    // Enable interrupt on the port
    //

    return ERR_NONE;
}

status_t
Sata::Close(UInt dev)
{
    return ERR_NONE;
}

status_t
Sata::NonData(UInt port, AtaCommandBlock *cb)
{
    return ERR_NONE;
}

// PIO Read
//XXX Note: length must be < 4MB
status_t
Sata::DataIn(UInt port, AtaCommandBlock *cb, void *buffer, size_t length)
{
    AhciCommandHeader *header;
    AhciCommandTable *table;
    AhciReceivedFis *fis;
    PhysicalRegionDescriptor *prd;  // Shdow Data Register
    UInt slot;

    ENTER;

    slot = _ahci.AllocateSlot(port);

    // Get the header and table
    header = _ahci.CommandHeader(port, slot);
    table = _ahci.CommandTable(port, slot);
    fis = _ahci.ReceivedFis(port);
    fis->Clear();

    // Fill up the command FIS with the command block
    table->BuildCommandFis(cb);

    // Set up physical region descriptors
    prd = table->Prdt();
    prd[0].SetDataBlock(GetPhys((unsigned long)buffer), length);

    //
    // Read mode
    //
    header->ClearDeviceWrite();

    header->SetPrdtl(1);
    header->SetPrefetchable();
    header->SetPmp(0);
    header->SetCommandFisLength(AhciCommandTable::CFIS_LENGTH);

    // Activate the command
    _ahci.IssueCommand(port, slot);

    L4_Receive(GetIntrThread());

    prd[0].UnsetDataBlock();

    EXIT;
    return ERR_NONE;
}

// PIO Write
//XXX: count must be < 4MB
status_t
Sata::DataOut(UInt port, AtaCommandBlock *cb, const void *buffer, size_t count)
{
    UInt                slot;
    AhciCommandHeader           *header;
    AhciCommandTable            *table;
    AhciReceivedFis             *fis;
    PhysicalRegionDescriptor    *prd;
    ENTER;
 
    slot = _ahci.AllocateSlot(port);

    // Get the header and table
    header = _ahci.CommandHeader(port, slot);
    table = _ahci.CommandTable(port, slot);
    fis = _ahci.ReceivedFis(port);
    fis->Clear();

    // Fill up the command FIS with the command block
    table->BuildCommandFis(cb);

    // Set up physical region descriptors
    prd = table->Prdt();
    prd[0].SetDataBlock(GetPhys((unsigned long)buffer), count);

    //
    // Write mode
    //
    header->SetDeviceWrite();

    header->SetPrdtl(1);
    //header->SetPrefetchable();
    header->SetPmp(0);
    header->SetCommandFisLength(AhciCommandTable::CFIS_LENGTH);

    // Activate the command
    _ahci.IssueCommand(port, slot);

    //L4_Receive(GetIntrThread());
    //prd[0].UnsetDataBlock();

    EXIT;
    return ERR_NONE;
}

// DMA Read
//XXX: count must be < 4MB
status_t
Sata::Read(UInt port, AtaCommandBlock *cb, void *buffer, size_t length)
{
    AhciCommandHeader           *header;
    AhciCommandTable            *table;
    AhciReceivedFis             *fis;
    PhysicalRegionDescriptor    *prd;
    UInt                slot;
    ENTER;
 
    slot = _ahci.AllocateSlot(port);

    // Get the header and table
    header = _ahci.CommandHeader(port, slot);
    table = _ahci.CommandTable(port, slot);
    fis = _ahci.ReceivedFis(port);
    fis->Clear();

    // Fill up the command FIS with the command block
    table->BuildCommandFis(cb);

    // Set up physical region descriptors
    prd = table->Prdt();
    prd[0].SetDataBlock(GetPhys((addr_t)buffer), length);
    //DOUT("phys: %.8lX\n", GetPhys((unsigned long)buffer));

    //
    // Turn it to read mode
    //
    header->ClearDeviceWrite();

    header->SetPrdtl(1);
    header->SetPrefetchable();
    header->SetPmp(0);
    header->SetCommandFisLength(AhciCommandTable::CFIS_LENGTH);

    // Activate the slot
    _ahci.IssueCommand(port, slot);

    L4_Receive(GetIntrThread());
    prd[0].UnsetDataBlock();

    EXIT;
    return ERR_NONE;
}

// DMA Write
//XXX: count must be < 4MB
status_t
Sata::Write(UInt port, AtaCommandBlock *cb, const void *buf, size_t count)
{
    AhciCommandHeader           *header;
    AhciCommandTable            *table;
    AhciReceivedFis             *fis;
    PhysicalRegionDescriptor    *prd;
    UInt                slot;

    ENTER;
 
    slot = _ahci.AllocateSlot(port);

    // Get the header and table
    header = _ahci.CommandHeader(port, slot);
    table = _ahci.CommandTable(port, slot);
    fis = _ahci.ReceivedFis(port);
    fis->Clear();

    // Fill up the command FIS with the command block
    table->BuildCommandFis(cb);

    // Set up physical region descriptors
    prd = table->Prdt();
    prd[0].SetDataBlock(GetPhys((unsigned long)buf), count);

    //
    // Turn it to write mode
    //
    header->SetDeviceWrite();

    header->SetPrdtl(1);
    //header->SetPrefetchable();
    header->SetPmp(0);
    header->SetCommandFisLength(AhciCommandTable::CFIS_LENGTH);

    // Activate the slot
    _ahci.IssueCommand(port, slot);

    //L4_Receive(GetIntrThread());
    //prd[0].UnsetDataBlock();

    EXIT;
    return ERR_NONE;
}

void
Sata::DumpStatus(UInt port)
{
    volatile UInt data;

    printf("-- stat port %lu\n", port);
    data = _ahci.ReadControl(ABAR_IS);
    printf("Interrupt status (IS):         %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXCMD);
    printf("Command register (PXCMD):      %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXSSTS);
    printf("Status register (PXSSTS):      %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXTFD);
    printf("Task file register (PXTFD):    %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXIS);
    printf("Port interrupt status (PXIS):  %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXSERR);
    printf("Error register (PXSERR):       %.8lX\n", data);
    data = _ahci.ReadPort(port, ABAR_PXCI);
    printf("Command issue register (PXCI): %.8lX\n", data);

    printf("-- stat end\n");
}

void
Sata::DumpCommandFis(AhciCommandTable *table)
{
    UByte *ptr;

    ptr = (UByte *)table->CommandFis();
    printf("Command FIS @ %p\n", ptr);
    for (int i = 0; i < 64; i += 8) {
        printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3],
               ptr[i + 4], ptr[i + 5], ptr[i + 6], ptr[i + 7]);
    }
}

void
Sata::DumpCommandHeader(AhciCommandHeader *header)
{
    UByte *ptr;

    ptr = (unsigned char *)header;
    printf("Command header\n");
    for (int i = 0; i < 16; i += 4) {
        printf("%.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }
}

void
Sata::DumpReceivedFis(AhciReceivedFis *fis)
{
    UByte *ptr;

    printf("DMA Setup FIS\n");
    ptr = (UByte *)fis->dmaSetup;
    for (UInt i = 0; i < AhciReceivedFis::DSFIS_LENGTH; i += 4) {
        printf("%.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }

    printf("\nPIO Setup FIS\n");
    ptr = (UByte *)fis->pioSetup;
    for (UInt i = 0; i < AhciReceivedFis::PSFIS_LENGTH; i += 4) {
        printf("%.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }

    printf("\nD2H Register FIS\n");
    ptr = (UByte *)fis->d2hRegister;
    for (UInt i = 0; i < AhciReceivedFis::RFIS_LENGTH; i += 4) {
        printf("%.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }

    printf("\nSet Device Bits FIS\n");
    ptr = (UByte *)fis->deviceBits;
    for (UInt i = 0; i < AhciReceivedFis::SDBFIS_LENGTH; i += 4) {
        printf("%.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }

    printf("\nUnknown FIS\n");
    ptr = (UByte *)fis->unknown;
    for (UInt i = 0; i < AhciReceivedFis::UFIS_LENGTH; i += 8) {
        printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
               ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3],
               ptr[i + 4], ptr[i + 5], ptr[i + 6], ptr[i + 7]);
    }
    printf("\n");
}

void
Sata::PrintDeviceInfo(unsigned short *data)
{
    char    *ptr;

    printf("%s%s%s\n",
           (data[0] & (1 << 15)) ? "ata " : "",
           (data[0] & (1 << 7)) ? "removable " : "",
           (data[0] & (1 << 2)) ? "inresp ": "");

    printf("Serial Number:   ");
    ptr = (char *)&data[10];
    for (int i = 0; i < 20; i += 2) {
        printf("%c%c", ptr[i + 1], ptr[i]);
    }
    printf("\n");

    printf("Firmware Rev:    ");
    ptr = (char *)&data[23];
    for (int i = 0; i < 8; i += 2) {
        printf("%c%c", ptr[i + 1], ptr[i]);
    }
    printf("\n");

    printf("Model Number:    ");
    ptr = (char *)&data[27];
    for (int i = 0; i < 40; i += 2) {
        printf("%c%c", ptr[i + 1], ptr[i]);
    }
    printf("\n");

    printf("Max Xfer Sectors: %X\n", data[59] & 0xF);

    printf("%s%s%s%s%s\n",
           (data[49] & (1 << 13)) ? "standby " : "",
           (data[49] & (1 << 11)) ? "iordy" : "",
           (data[49] & (1 << 10)) ? "(disabled) " : " ",
           (data[49] & (1 << 9)) ? "lba " : "",
           (data[49] & (1 << 8)) ? "dma " : "");

    printf("Multiword DMA:   %X\n", data[63]);
    printf("DMA cycletime:   %u (%u)\n", data[66], data[65]);
    printf("PIO cycletime:   %u (%u)\n", data[67], data[68]);

    printf("Major version:   ");
    for (int i = 0; i < 16; i++) {
        if ((data[80] & (1 << i)) == (1 << i)) {
            printf("%d ", i + 1);
        }
    }
    printf("\n");
}

void
Sata::Test()
{
    AtaCommandBlock cb;
    UInt port = 1;


    cb.Initialize();
    cb.count = 4;
    cb.SetLba(0);

    for (int i = 0; i < BUFFER_SIZE; i++) {
        infoBuffer[i] = 0;
    }
    printf("SATA Test: DMA Read\n");
    cb.command = ATA_CMD_READ_DMA_R;
    Read(port, &cb, infoBuffer, BUFFER_SIZE);
    for (int i = 0; i < 512; i += 16) {
        printf("%.8X ", i);
        for (int j = i; j < i + 16; j++) {
            printf("%.2X ", infoBuffer[j] & 0xFF);
        }
        printf("\n");
    }


    for (int i = 0; i < BUFFER_SIZE; i++) {
        infoBuffer[i] = 0;
    }
    printf("SATA Test: Data In\n");
    cb.command = ATA_CMD_READ_SECTOR;
    DataIn(port, &cb, infoBuffer, BUFFER_SIZE);
    for (int i = 0; i < 512; i += 16) {
        printf("%.8X ", i);
        for (int j = i; j < i + 16; j++) {
            printf("%.2X ", infoBuffer[j] & 0xFF);
        }
        printf("\n");
    }

    printf("SATA Test: End\n");
}

int
main()
{
    ConsoleOut(INFO, "Starting SATA driver (%.8lX) ...\n", L4_Myself().raw);

    _main = L4_Myself();

    _driver.Initialize();
    StartDevice(&_driver);

    return 0;
}

