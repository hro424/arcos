/*
 *
 *  Copyright (C) 2007, 2008, Waseda University. All rights reserved.
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
/// @file   Services/Devices/Disk/Pata/AtaCommandBlock.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: AtaCommandBlock.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICES_ATA_COMMAND_BLOCK_H
#define ARC_DEVICES_ATA_COMMAND_BLOCK_H

#include <Types.h>
#include "Ata.h"

class AtaCommandBlock {
public:
    UShort      features;       // Features
    UShort      count;          // Sector count
    struct {
        UShort  lo;
        UShort  mid;
        UShort  hi;
    } lba;
    UByte       device;         // Device/Head
    UByte       command;        // contents of the command register
    UByte       control;        // contents of the device control reg

    void Initialize() {
        features = 0;
        count = 0;
        lba.lo = 0;
        lba.mid = 0;
        lba.hi = 0;
        device = 1 << 7 | 1 << 5;
        command = 0;
        control = 0;
    }

    void SetLba(unsigned int val) {
        lba.lo = (UShort)(val & 0xFF);
        lba.mid = (UShort)((val >> 8) & 0xFF);
        lba.hi = (UShort)((val >> 16) & 0xFF);
        device &= ~0x0F;
        device |= (UByte)((val >> 24) & 0x0F);
        device |= ATA_REG_DH_LBA;
    }

    unsigned int Lba() {
        return (unsigned int)lba.lo | ((unsigned int)lba.mid << 8) |
            ((unsigned int)lba.hi << 16) |
            (((unsigned int)device & 0xF) << 24);
    }

    Bool IsLba() {
        return ((device & ATA_REG_DH_LBA) == ATA_REG_DH_LBA);
    }

    void Device0() {
        device &= ~ATA_REG_DH_DEV;
    }

    void Device1() {
        device |= ATA_REG_DH_DEV;
    }
};

#endif // ARC_DEVICES_SATA_ATA_COMMAND_BLOCK_H

