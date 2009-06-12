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
/// @file   Devices/Disk/Pata/Port.cc
/// @brief  Port operations implementation
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: Port.cc 429 2008-11-01 02:24:02Z hro $

/*
 * Reference:
 * [1] Peter L. McLean. AT Attachment-3 Interface, X3T13, January 1997.
 */

#include <Debug.h>
#include <Assert.h>
#include <arc/IO.h>
#include <Interrupt.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <Types.h>
#include <l4/types.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include "Ata.h"
#include "AtaCommandBlock.h"
#include "Port.h"


const UInt Port::CommandRegister[NUM_PORTS] =
        {IO_PIIX_PIDE_CMD, IO_PIIX_SIDE_CMD};
const UInt Port::ControlRegister[NUM_PORTS] =
        {IO_PIIX3_PIDE_CTRL, IO_PIIX3_SIDE_CTRL};

const addr_t Port::ChannelBase[NUM_PORTS] = { 0x00, 0x08 };

UByte
Port::ReadCommand(CommandRegisterOffset reg)
{
    return inb(CommandRegister[_port] + reg);
}

void
Port::WriteCommand(CommandRegisterOffset reg, UByte val)
{
    outb(CommandRegister[_port] + reg, val);
}

UByte
Port::ReadControl(ControlRegisterOffset reg)
{
    return inb(ControlRegister[_port] + reg);
}

void
Port::WriteControl(ControlRegisterOffset reg, UByte val)
{
    outb(ControlRegister[_port] + reg, val);
}

UInt
Port::ReadData(addr_t buffer, size_t count)
{
    return IO_Read16(CommandRegister[_port] + CMD_DATA,
                     reinterpret_cast<void *>(buffer), count);
}

UInt
Port::WriteData(const void *buffer, size_t count)
{
    return IO_Write16(CommandRegister[_port] + CMD_DATA, buffer, count);
}

void
Port::IssueCommand(AtaCommandBlock *cb)
{
    UByte   stat;

    ENTER;

    // For interrupt handling
    _self = L4_Myself();

    // Read status register
    // Wait for the device not busy
    do {
        stat = ReadControl(CTRL_ASTAT);
    } while ((stat & ATA_REG_STS_BSY) != 0);

    // Write the device/head register w/appropriate DEV bit value
    WriteCommand(CMD_DEV_HEAD, cb->device);

    // Read the status register
    // Wait for the device bocomes not busy and ready
    do {
        stat = ReadControl(CTRL_ASTAT);
    } while (((stat & ATA_REG_STS_BSY) != 0)
             || ((stat & ATA_REG_STS_DRDY) != ATA_REG_STS_DRDY));

    WriteCommand(CMD_FEATURE, (UByte)(cb->features & 0xFF));
    WriteCommand(CMD_SECT_CNT, (UByte)(cb->count & 0xFF));

    if (cb->IsLba()) {
        // LBA mode
        WriteCommand(CMD_LBA_LO, (UByte)(cb->lba.lo & 0xFF));
        WriteCommand(CMD_LBA_MID, (UByte)(cb->lba.mid & 0xFF));
        WriteCommand(CMD_LBA_HI, (UByte)(cb->lba.hi & 0xFF));
        WriteCommand(CMD_DEV_HEAD, cb->device);
    }
    /*
    else {
    	// CHS mode
    	WritePortCommand(port, ATA_CMD_SECT_NUM, cb->Y.chs.sectorNumber);
    	WritePortCommand(port, ATA_CMD_CYL_LO, cb->Y.chs.cylinderLow);
    	WritePortCommand(port, ATA_CMD_CYL_HI, cb->Y.chs.cylinderHigh);
    	WritePortCommand(port, ATA_CMD_DEV_HEAD, cb->deviceHead);
    }
    */
    DOUT("Command: 0x%.2X\n", cb->command);
    DOUT("Status: 0x%.2X\n", Status());
    WriteCommand(CMD_COMMAND, cb->command);

    int i = 0;
    while (i < 1000) i++;

    DOUT("Status: 0x%.2X\n", Status());
    EXIT;
}

UByte
Port::WaitDevice()
{
    UByte   stat;
    ENTER;

    if (IsInterruptEnable()) {
        DOUT("wait: %.8lX\n", this->Id().raw);
        L4_Receive(this->Id());
    }
    else {
        do {
            stat = ReadControl(CTRL_ASTAT);
        } while ((stat & ATA_REG_STS_BSY) != 0);
    }

    EXIT;
    return Status();
}

UByte
Port::Status()
{
    return ReadCommand(CMD_STATUS);
}

UByte
Port::ErrorCode()
{
    return ReadCommand(CMD_ERROR);
}

void
Port::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    stat_t err;
    ENTER;
    //DOUT("To %.8lX\n", _self.raw);
    err = Ipc::Send(_self, msg);
    if (err != ERR_NONE) {
        BREAK("IDE: Err_HandleInterrupt");
    }
    EXIT;
}

Bool
Port::EnableInterrupt()
{
    ENTER;

    InterruptManager* imng = InterruptManager::Instance();
    imng->Register(this, _port == 0 ? IRQ_PIDE : IRQ_SIDE);
    _int_enabled = TRUE;

    EXIT;
    return TRUE;
}

void
Port::DisableInterrupt()
{
    ENTER;

    InterruptManager* imng = InterruptManager::Instance();
    imng->Deregister(_port == 0 ? IRQ_PIDE : IRQ_SIDE);
    _int_enabled = FALSE;

    EXIT;
}

Bool
Port::IsInterruptEnable()
{
    return _int_enabled;
}

UByte
Port::ReadBMIRegister(UInt channel, addr_t reg)
{
    return *(UByte*)(_bmi_base + ChannelBase[channel] + reg);
}

void
Port::WriteBMIRegister(UInt channel, addr_t reg, UByte value)
{
    *(volatile UByte*)(_bmi_base + ChannelBase[channel] + reg) = value;
}

UByte
Port::ReadBMICommand(UInt channel)
{
    return ReadBMIRegister(channel, 0);
}

void
Port::WriteBMICommand(UInt channel, UByte value)
{
    WriteBMIRegister(channel, 0, value);
}

UByte
Port::ReadBMIStatus(UInt channel)
{
    return ReadBMIRegister(channel, 0x02);
}

void
Port::WriteBMIStatus(UInt channel, UByte value)
{
    WriteBMIRegister(channel, 0x02, value);
}

addr_t
Port::ReadPRD(UInt channel)
{
    return *(addr_t*)(_bmi_base + ChannelBase[channel] + 0x04);
}

void
Port::WritePRD(UInt channel, addr_t prd)
{
    *(volatile addr_t*)(_bmi_base + ChannelBase[channel] + 0x04) = prd;
}

#define BMI_BASE        0xe800

Bool
Port::EnableDMA()
{
    UByte   reg;

    ENTER;

    // Disable I/O space to set up the bus master interface address.
    reg = PCI_Read16(PIIX3_IDE, PCI_PCICMD);
    DOUT("pcicmd %.4lX\n", reg);
    PCI_Write16(PIIX3_IDE, PCI_PCICMD, reg & ~1);
    reg = PCI_Read16(PIIX3_IDE, PCI_PCICMD);
    DOUT("pcicmd %.4lX\n", reg);

    // Set bus master interface address
    DOUT("bmiba %.8lX\n", PCI_Read32(PIIX3_IDE, PCI_BAR));
    PCI_Write32(PIIX3_IDE, PCI_BAR, BMI_BASE | 1);
    DOUT("bmiba %.8lX\n", PCI_Read32(PIIX3_IDE, PCI_BAR));

    // Enable UDMA
    //reg = PCI_Read8(PIIX3_IDE, PCI_SDMA_CNT);
    //TODO
    //reg |= (1 << _port) | drive;
    //reg |= 1 << _port;
    //PCI_Write8(PIIX3_IDE, PCI_SDMA_CNT, reg);

    // Enable bus master and I/O space
    PCI_Write16(PIIX3_IDE, PCI_PCICMD, (1 << 2) | 1 );

    // Stop DMA
    //WriteBMICommand(_port, 0);

    DOUT("pcicmd %.4lX\n", PCI_Read16(PIIX3_IDE, PCI_PCICMD));
    DOUT("BMI comm %.2lX\n", inb(BMI_BASE));
    //DOUT("(BMI comm) %.2X\n", ReadBMICommand(_port));

    // Map PRD space.  Its physical address has to be 64kbyte aligned.
    _prd_virt = palloc(1);
    Pager.Map(_prd_virt, L4_ReadWriteOnly, 0x00200000, L4_nilthread);

    EXIT;
    return true;
}

void
Port::DisableDMA()
{
    UByte   reg;

    ENTER;

    reg = PCI_Read16(PIIX3_IDE, PCI_PCICMD);
    PCI_Write16(PIIX3_IDE, PCI_PCICMD, reg & ~1);

    pfree(_prd_virt, 1);

    EXIT;
}

addr_t
Port::SetPhysicalRegionDescriptor(addr_t base, size_t count)
{
    UInt*   ptr = reinterpret_cast<UInt*>(_prd_virt);
    //TODO: count >= 64KB
    ptr[0] = base & 1;
    ptr[1] = ((1 << 31) | count) & ~1;
    DOUT("prd[0] %.8lX prd[1] %.8lX\n", ptr[0], ptr[1]);
    //return Pager.Phys(_prd_virt);
    return 0x00200000;
}

void
Port::SetDMAChannel(AtaCommandBlock* cb, addr_t base)
{
    addr_t  prd_phys;
    UByte   reg;
    ENTER;

    prd_phys = SetPhysicalRegionDescriptor(Pager.Phys(base),
                                           cb->count * ATA_SECTOR_SIZE);
    // Set up PRD (physical region descriptor)
    //WritePRD(_port, prd_phys);
    outl(BMI_BASE + 4, prd_phys);
    DOUT("prd phys %.8lX\n", inl(BMI_BASE + 4));

    // Set Read/Write Control bit. Bus master must be inactive during the
    // operation.
    if (cb->command != ATA_CMD_WRITE_DMA) {
        outb(BMI_BASE, 1 << 3);
    }

    DOUT("BMI comm %.2lX\n", inb(BMI_BASE));
    DOUT("BMI stat %.2lX\n", inb(BMI_BASE + 2));
    // Clear the Interrupt bit and Error bit in the Statue register.
    reg = inb(BMI_BASE + 2);
    outb(BMI_BASE + 2, reg | (1 << 2) | (1 << 1));
    DOUT("BMI stat %.2lX\n", inb(BMI_BASE + 2));

    EXIT;
}

void
Port::EngageDMA()
{
    ENTER;
    // Write a 1 to the Start bit in the Bus Master IDE Command Register
    UByte reg = inb(BMI_BASE);
    DOUT("BMI comm %.2lX\n", reg);
    outb(BMI_BASE, reg | 1);
    DOUT("BMI comm %.2lX\n", inb(BMI_BASE));
    //do {
        reg = inb(BMI_BASE + 2);
    //} while ((reg & 0x1) == 0);
    DOUT("BMI stat %.2lX\n", reg);

    EXIT;
}

void
Port::ResetDMAChannel()
{
    ENTER;
    // Reset the Start/Stop bit in the comand register
    outb(BMI_BASE + 2, (1 << 5) | (1 << 2) | (1 << 1));
    DOUT("BMI stat %.2lX\n", inb(BMI_BASE + 2));
    UByte reg = inb(BMI_BASE);
    outb(BMI_BASE, reg & ~1);
    DOUT("BMI stat %.2lX\n", inb(BMI_BASE + 2));
    EXIT;
}

Port::Port(UInt port)
{
    ENTER;

    _port = port;
    _int_enabled = FALSE;
    if (!SoftReset()) {
        System.Print(System.WARN, "disk iface %lu reset failed\n", port);
    }

    EXIT;
}

//---------------------------------------------------------------------------
//	Protocols
//	@see pp.110 in [1]
//---------------------------------------------------------------------------

Bool
Port::SoftReset()
{
    UByte stat;
    long counter;

    // Set SRST. BSY will be set in 400ns.
    WriteControl(CTRL_DEVICE, ATA_REG_DC_SRST);
    counter = 0;
    do {
        stat = ReadControl(CTRL_ASTAT);
        if (counter > 400) {
            break;
        }
        counter++;
    } while ((stat & ATA_REG_STS_BSY) == 0);

    // Read the error register
    UByte err = ErrorCode();
    if (err != 0) {
        return false;
    }

    // Clear SRST. BSY will be cleared in 31s. DRDY is supposed to be 1 at that
    // moment.
    WriteControl(CTRL_DEVICE, 0);
    counter = 0;
    do {
        stat = ReadControl(CTRL_ASTAT);
        /*
        if (counter > 31000000000) {
            return false;
        }
        counter++;
        */
    } while (((stat & ATA_REG_STS_BSY) != 0)
             || ((stat & ATA_REG_STS_DRDY) != ATA_REG_STS_DRDY));

    return true;
}

//
// Implementation of the ATA protocol for PIO data in commands
//
// This class includes:
//   - Identify device
//   - Read buffer
//   - Read multiple
//   - Read sector(s)
//   - Smart read attribute thresholds
//   - Smart read attribute values
// Execution of this class of command includes the transfer of one or more
// blocks of data from the device to the host.
//
UByte
Port::DataIn(AtaCommandBlock *cb, addr_t buffer)
{
    addr_t  ptr;
    UByte   stat;
    UByte   err;
    size_t  size;

    ENTER;

    ptr = buffer;

    IssueCommand(cb);

    do {
        stat = WaitDevice();

        if ((stat & ATA_REG_STS_DRQ) == ATA_REG_STS_DRQ) {
            // Transfer data from device
            size = ReadData(ptr, cb->count * ATA_SECTOR_SIZE);
            ptr += size;
        }

        assert((ptr - buffer) <=
               static_cast<UInt>(cb->count) * ATA_SECTOR_SIZE);

        err = ErrorCode();
    } while (ptr - buffer < static_cast<UInt>(cb->count) * ATA_SECTOR_SIZE &&
             (stat & ATA_REG_STS_DRQ) == ATA_REG_STS_DRQ &&
             err == 0);

    EXIT;
    return err;
}

//
// Implementation of the ATA protocol for PIO data out commands
//
// This class includes:
//   - Download microcode
//   - Format track
//   - Security disable password
//   - Security erase unit
//   - Security set password
//   - Serucity unlock
//   - Write buffer
//   - Write multiple
//   - Write sector(s)
//   - Write verify
//
UByte
Port::DataOut(AtaCommandBlock *cb, const void *buffer)
{
    UByte   stat;
    UByte   err;

    IssueCommand(cb);

    do {
        stat = WaitDevice();

        if (stat & ATA_REG_STS_DRQ) {
            // Transfer data to device
            WriteData(buffer, cb->count * ATA_SECTOR_SIZE);
        }
        err = ErrorCode();
    } while ((stat & ATA_REG_STS_DRQ) == ATA_REG_STS_DRQ);

    return err;
}

//
// Implementation of the ATA protocol for non-data commands
//
// This class includes:
//   - Check power mode
//   - Door lock
//   - Door unlock
//   - Execute device diagnostic
//   - Idle
//   - Idle immediate
//   - Initialize device parameters
//   - Media eject
//   - Nop
//   - Read verify sector(s)
//   - Recalibrate
//   - Security erase prepare
//   - Security freeze lock
//   - Seek
//   - Set features
//   - Set multiple mode
//   - Sleep
//   - SMART disable operation
//   - SMART enable/disable autosave
//   - SMART enable operation
//   - SMART return status
//   - SMART save attribute values
//   - Standby
//   - Stanby imeediate
// Execution of these commands involves no data transfer.
//
UByte
Port::NonData(AtaCommandBlock *cb)
{
    IssueCommand(cb);

    WaitDevice();

    return ErrorCode();
}

//
// Implementation of the ATA protocol for DMA data transfer
//
// This class comprises:
//   - Read DMA
//   - Write DMA
//   - Identify device DMA
//
UByte
Port::DMATransfer(AtaCommandBlock *cb, addr_t buffer)
{
    UByte   stat;
    ENTER;
    
    SetDMAChannel(cb, buffer);

    DOUT("Status: 0x%.2X\n", Status());
    IssueCommand(cb);

    DOUT("Status: 0x%.2X\n", Status());
    EngageDMA();
    DOUT("Status: 0x%.2X\n", Status());

    stat = WaitDevice();
    DOUT("Status: 0x%.2X\n", stat);

    ResetDMAChannel();

    EXIT;
    return ErrorCode();
}

