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
/// @file   Services/Devices/Sata/AhciCommandTable.cc
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

//$Id: AhciCommandTable.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/types.h>
#include "AhciCommandTable.h"

void *
AhciCommandTable::CommandFis()
{
    return _fis;
}

void *
AhciCommandTable::AtapiCommand()
{
    return _atapi;
}

PhysicalRegionDescriptor *
AhciCommandTable::Prdt()
{
    //return (PhysicalRegionDescriptor *)&_prdt;
    return _prdt;
}

status_t
AhciCommandTable::BuildCommandFis(AtaCommandBlock *cb)
{
    _fis[0] = 0x27;
    _fis[1] = (UByte)1 << 7;
    _fis[2] = cb->command;
    _fis[3] = (UByte)(cb->features & 0xFF);
    _fis[4] = (UByte)(cb->lba.lo & 0xFF);
    _fis[5] = (UByte)(cb->lba.mid & 0xFF);
    _fis[6] = (UByte)(cb->lba.hi & 0xFF);
    _fis[7] = cb->device;
    _fis[8] = (UByte)((cb->lba.lo >> 8) & 0xFF);
    _fis[9] = (UByte)((cb->lba.mid >> 8) & 0xFF);
    _fis[10] = (UByte)((cb->lba.hi >> 8) & 0xFF);
    _fis[11] = (UByte)((cb->features >> 8) & 0xFF);
    _fis[12] = (UByte)(cb->count & 0xFF);
    _fis[13] = (UByte)((cb->count >> 8) & 0xFF);
    _fis[14] = 0;
    _fis[15] = cb->control;
    _fis[16] = 0;
    _fis[17] = 0;
    _fis[18] = 0;
    _fis[19] = 0;

    return ERR_NONE;
}

void
AhciCommandTable::IssueCommand(AtaCommandBlock *cb)
{
    _fis[2] = cb->command;
}


