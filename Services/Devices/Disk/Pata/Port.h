/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
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
/// @file   Devices/Ide/Port.h
/// @brief  Ata port operations
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
/// @see    Peter L. McLean. AT Attachment-3 Interface, X3T13, January 1997.
///

//$Id: Port.h 392 2008-09-02 11:49:51Z hro $

#ifndef ARC_DEVICES_IDE_PORT_H
#define ARC_DEVICES_IDE_PORT_H

#include <Interrupt.h>
#include <Types.h>
#include <l4/types.h>
#include "AtaCommandBlock.h"

class Port : public InterruptHandler
{
public:
    static const UInt NUM_PORTS = 2;

private:
    enum CommandRegisterOffset
    {
        CMD_DATA =      0,
        CMD_ERROR =     1,
        CMD_FEATURE =   1,
        CMD_SECT_CNT =  2,
        CMD_SECT_NUM =  3,
        CMD_LBA_LO =    3,
        CMD_CYL_LO =    4,
        CMD_LBA_MID =   4,
        CMD_CYL_HI =    5,
        CMD_LBA_HI =    5,
        CMD_DEV_HEAD =  6,
        CMD_STATUS =    7,
        CMD_COMMAND =   7
    };

    enum ControlRegisterOffset
    {
        CTRL_ASTAT =    0,
        CTRL_DEVICE =   6,
    };

    static const UInt IRQ_PIDE = 14;
    static const UInt IRQ_SIDE = 15;

    static const UInt   CommandRegister[NUM_PORTS];
    static const UInt   ControlRegister[NUM_PORTS];
    static const addr_t ChannelBase[NUM_PORTS];

    UInt                _port;

    L4_ThreadId_t       _self;

    Bool                _int_enabled;

    addr_t              _bmi_base;

    addr_t              _prd_virt;

    addr_t              _prd_phys;

    UByte ReadCommand(CommandRegisterOffset reg);
    void WriteCommand(CommandRegisterOffset reg, UByte val);
    UByte ReadControl(ControlRegisterOffset reg);
    void WriteControl(ControlRegisterOffset reg, UByte val);

    ///
    /// Delivers the command to the device.
    ///
    void IssueCommand(AtaCommandBlock *cb);

    ///
    /// Reads data from the device.  The device must be preliminary in the
    /// appropriate state by means of the functions above.  See [1].
    ///
    UInt ReadData(addr_t buffer, size_t count);

    ///
    /// Writes the data to the device.  The device must be preliminary in the
    /// appropriate state by means of the functions above.  See [1].
    ///
    UInt WriteData(const void *buffer, size_t count);

    ///
    /// Obtains the status of the port.
    ///
    UByte Status();

    ///
    /// Obtains the error status of the port.
    ///
    UByte ErrorCode();

    Bool SoftReset();

    ///
    /// Waits for the device finish the command.
    ///
    UByte WaitDevice();

    UByte ReadBMIRegister(UInt channel, addr_t reg);
    void WriteBMIRegister(UInt channel, addr_t reg, UByte value);
    UByte ReadBMICommand(UInt channel);
    void WriteBMICommand(UInt channel, UByte value);
    UByte ReadBMIStatus(UInt channel);
    void WriteBMIStatus(UInt channel, UByte value);
    addr_t ReadPRD(UInt channel);
    void WritePRD(UInt channel, addr_t prd);

    addr_t SetPhysicalRegionDescriptor(addr_t base, size_t count);
    void SetDMAChannel(AtaCommandBlock* cb, addr_t base);
    void EngageDMA();
    void ResetDMAChannel();

public:
    Port(UInt port);

    UInt GetPortNumber() { return _port; }

    Bool EnableInterrupt();

    void DisableInterrupt();

    Bool IsInterruptEnable();

    Bool EnableDMA();

    void DisableDMA();

    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg);

    UByte DataIn(AtaCommandBlock *cb, addr_t buffer);

    UByte DataOut(AtaCommandBlock *cb, const void *buffer);

    UByte NonData(AtaCommandBlock *cb);

    UByte DMATransfer(AtaCommandBlock *cb, addr_t buffer);
};

#endif // ARC_DEVICES_IDE_PORT_H

