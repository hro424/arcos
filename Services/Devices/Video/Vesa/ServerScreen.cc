/*
 *
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
/// @brief  Defines the VbeInfoBlock VESA VBE 2.0 structure.
/// @file   Services/Devices/Vesa/VbeInfoBlock.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///

//$Id: ServerScreen.cc 349 2008-05-29 01:54:02Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include <String.h>
#include <System.h>
#include <Types.h>
#include <x86emu.h>

#include "ServerScreen.h"
#include "ServerVideoMode.h"
#include "ModeInfoBlock.h"
#include "VbeInfoBlock.h"
#include "RealModeEmu.h"

static const struct ModeInfoBlock *const
queryVideoMode(UShort number)
{
    X86EMU_SETREG(AX, 0x4f01);
    X86EMU_SETREG(CX, number);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, sizeof(struct VbeInfoBlock));
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print(System.ERROR, "vesa: Cannot query mode %x!\n", number);
        return 0;
    }
    const struct ModeInfoBlock *const modeInfo = 
        (const struct ModeInfoBlock *const) (emu_main_mem + X86EMU_DATA_AREA_OFFSET + 
                sizeof(struct VbeInfoBlock));
    return modeInfo;
}

stat_t
ServerScreen::Initialize()
{
    ServerVideoMode*      mode;
    stat_t  err;
   
    err = realModeEmu_init();
    if (err != ERR_NONE) {
        System.Print(System.ERROR,
                     "vesa: Cannot initialize x86 emulation layer!\n");
        return ERR_NOT_FOUND;
    }
    
    // First get the address of the VBE info block.
    X86EMU_SETREG(AX, 0x4f00);
    X86EMU_SETREG(ES, (X86EMU_DATA_AREA_OFFSET) >> 4);
    X86EMU_SETREG(DI, 0);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print(System.ERROR, "vesa: Cannot query VBE capabilities!\n");
        return ERR_NOT_FOUND;
    }
    
    // Get the VBE information block (read only)
    const VbeInfoBlock* const info =
        reinterpret_cast<const struct VbeInfoBlock* const>(
                                    emu_main_mem + X86EMU_DATA_AREA_OFFSET);
    
    if (memcmp(info->VbeSignature, "VESA", 4) &&
        memcmp(info->VbeSignature, "VBE2", 4)) {
        System.Print(System.ERROR,
                     "vesa: Vesa compatible board not detected!\n");
        return ERR_NOT_FOUND;
    }
    
    _version = info->VbeVersion;
    _total_memory = info->TotalMemory;
    
    // Also create the list of supported video modes - be careful not to
    // overflow our target
    mode = const_cast<ServerVideoMode*>(
                reinterpret_cast<const ServerVideoMode*>(GetSupportedModes()));
    _num_modes = 0;
    for (const UShort* ptr = reinterpret_cast<const UShort*>(
                            emu_main_mem + info->VideoModePtr.address());
         *ptr != 0xffff;
         ptr++) {
        const ModeInfoBlock* const block = queryVideoMode(*ptr);
        // Cannot query mode?
        if (!block) {
            continue;
        }
        // No hardware support?
        if (!(block->ModeAttributes & VIDEO_MODE_SUPPORTED)) {
            continue;
        }
        // Text mode?
        if (!(block->ModeAttributes & VIDEO_MODE_GRAPHIC)) {
            continue;
        }
        // No LFB support?
        if (!(block->ModeAttributes & VIDEO_MODE_SUPPORTS_LFB)) {
            continue;
        }

        mode[_num_modes].Initialize(*ptr, block);
        _num_modes++;
    }
    
    _current_mode = -1;
    return ERR_NONE;
}

stat_t
ServerScreen::CleanUp()
{
    return realModeEmu_cleanup();
}

stat_t
ServerScreen::SetVideoMode(UInt index)
{
    ENTER;

    stat_t      err;
    L4_Word_t   fb_base;
    UInt        num_pages;
    const ServerVideoMode& mode =
        ((const ServerVideoMode *)GetSupportedModes())[index];

    // Set the video mode using BIOS functions
    X86EMU_SETREG(AX, 0x4f02);
    X86EMU_SETREG(BX, mode.number() | VIDEO_MODE_USE_LFB);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        return ERR_UNKNOWN;
    }

    // Now map the frame buffer to our address space
    // We have to map the pages one by one - so let's first calculate how many
    // pages to map.
    fb_base = mode.lfbPhysicalAddress();
    num_pages = mode.LFBNbPages();
    _server_base = reinterpret_cast<UByte*>(palloc_shm(num_pages, L4_anythread,
                                                       L4_ReadWriteOnly));
    for (UInt i = 0; i < num_pages; i++) {
        err = Pager.Map(reinterpret_cast<addr_t>(_server_base) +
                            (PAGE_SIZE * i),
                        fb_base + (PAGE_SIZE * i), L4_nilthread, 1,
                        L4_FullyAccessible);
        if (err != ERR_NONE) {
            return err;
        }
    }
    
    _current_mode = index;
    _bpp = GetCurrentMode()->bytespp();
    _xres = GetCurrentMode()->xres();
    
    EXIT;
    return ERR_NONE;
}

