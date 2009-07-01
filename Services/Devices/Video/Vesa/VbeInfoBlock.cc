/*
 *  Copyright (C) 2008, Waseda University.
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
/// @file   Services/Devices/Vesa/VbeInfoBlock.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Defines the VbeInfoBlock VESA VBE 2.0 structure.
///

#include <Debug.h>
#include <System.h>
#include <String.h>

#include "VbeInfoBlock.h"
#include "RealModeEmu.h"

stat_t
ReadVbeInfoBlock(VbeInfoBlock* block)
{
    addr_t  block_addr;

    // First get the address of the VBE info block.
    X86EMU_SETREG(AX, 0x4f00);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, 0);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print("vesa: Cannot query VBE capabalities!\n");
        return ERR_NOT_FOUND;
    }
 
    memset(block, 0, sizeof(VbeInfoBlock));
    block_addr = _rme_base + X86EMU_DATA_AREA_OFFSET;
    memcpy(block, (const void*)block_addr, sizeof(VbeInfoBlock));
    return ERR_NONE;
}

