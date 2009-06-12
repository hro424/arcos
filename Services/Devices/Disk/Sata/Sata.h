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
/// @file   Services/Devices/SATA/Sata.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

//$Id: Sata.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_SATA_H
#define ARC_DEVICES_SATA_H

#include <arc/status.h>
#include <l4/message.h>
#include "Ahci.h"
#include "AtaCommandBlock.h"

class Sata {
private:
    enum {
        FIS_TYPE_REG_H2D =      0x27,
        FIS_TYPE_REG_D2H =      0x34,
        FIS_TYPE_DEVICE_BITS =  0xA1,
        FIS_TYPE_DMA_ACTIVATE = 0x39,
        FIS_TYPE_DMA_SETUP =    0x41,
        FIS_TYPE_BIST =         0x58,
        FIS_TYPE_PIO_SETUP =    0x5F,
        FIS_TYPE_DATA =         0x46,
    };

    Ahci _ahci;

    ///
    /// Initializes the address where the AHCI device is mapped.  The region
    /// doesn't overwrap with the main memory.
    ///
    status_t SetupAhci();

    void PrintDeviceInfo(unsigned short *data);
    void DumpStatus(UInt port);
    void DumpCommandFis(AhciCommandTable *table);
    void DumpCommandHeader(AhciCommandHeader *header);
    void DumpReceivedFis(AhciReceivedFis *fis);

    void HandleUnknownFis(UInt port);
    void HandleSetDeviceBits(UInt port);
    void HandlePioSetupFis(UInt port);
    void HandleDmaSetupFis(UInt port);
    void HandleDeviceToHostRegisterFis(UInt port);
    void HandleDataFis(UInt port);

public:
    status_t Initialize();
    void Init();
    void Test();

    status_t HandleInterrupts(L4_ThreadId_t tid, L4_Msg_t *msg);
    status_t Open(UInt port);
    status_t Close(UInt port);
    status_t NonData(UInt port, AtaCommandBlock *cb);
    status_t DataIn(UInt port, AtaCommandBlock *cb, void *buffer, size_t count);
    status_t DataOut(UInt port, AtaCommandBlock *cb, const void *buffer,
                     size_t count);
    status_t Read(UInt port, AtaCommandBlock *cb, void *buffer, size_t count);
    status_t Write(UInt port, AtaCommandBlock *cb, const void *buffer,
                   size_t count);
};

#endif // ARC_DEVICES_SATA_H

