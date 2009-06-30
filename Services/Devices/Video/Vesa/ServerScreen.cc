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
/// @file   Services/Devices/Vesa/VbeInfoBlock.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Defines the VbeInfoBlock VESA VBE 2.0 structure.

#include <arc/console.h>
#include <arc/system.h>
#include <arc/status.h>
#include <arc/memory.h>
#include <arc/ipc.h>

#include "ServerScreen.h"
#include "ServerVideoMode.h"
#include "ModeInfoBlock.h"
#include "VbeInfoBlock.h"
#include "RealModeEmu.h"

#include <l4/kip.h>

//UShort Screen::Version;
//UInt Screen::TotalMemory;
//UByte *const Screen::fb = (UByte *)VBE_LFB_ADDRESS;
//UByte Screen::bytespp = 0;
//UShort Screen::xres = 0;

//const VideoMode *Screen::supportedModes[MAX_SUPPORTED_MODES + 1];
//const VideoMode *Screen::currentMode = 0;

static const struct ModeInfoBlock *const
queryVideoMode(UShort number) {
    X86EMU_SETREG(AX, 0x4f01);
    X86EMU_SETREG(CX, number);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, sizeof(struct VbeInfoBlock));
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        ConsoleOut(ERROR, "vesa: Cannot query mode %x!\n", number);
        return 0;
    }
    const struct ModeInfoBlock *const modeInfo = 
        (const struct ModeInfoBlock *const) (emu_main_mem + X86EMU_DATA_AREA_OFFSET + 
                sizeof(struct VbeInfoBlock));
    return modeInfo;
}

status_t ServerScreen::init() {
    status_t err = realModeEmu_init();
    if (err != ERR_NONE) {
        ConsoleOut(ERROR, "vesa: Cannot initialize x86 emulation layer!\n");
        return ERR_NOT_FOUND;
    }
    
    // First get the address of the VBE info block.
    X86EMU_SETREG(AX, 0x4f00);
    X86EMU_SETREG(ES, (X86EMU_DATA_AREA_OFFSET) >> 4);
    X86EMU_SETREG(DI, 0);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        ConsoleOut(ERROR, "vesa: Cannot query VBE capabilities!\n");
        return ERR_NOT_FOUND;
    }
    
    // Get the Vbe Info Block
    const struct VbeInfoBlock *const infoBlock = 
        (const struct VbeInfoBlock *const) (emu_main_mem + X86EMU_DATA_AREA_OFFSET);
    
    if (memcmp(infoBlock->VbeSignature, "VESA", 4) &&
            memcmp(infoBlock->VbeSignature, "VBE2", 4)) {
        ConsoleOut(ERROR, "vesa: Vesa compatible board not detected!\n");
        return ERR_NOT_FOUND;
    }
    
    Version = infoBlock->VbeVersion;
    TotalMemory = infoBlock->TotalMemory;
    
    // Also create the list of supported video modes - be careful not to overflow 
    // our target
    UInt modeIndex = 0;
    NbSupportedModes = 0;
    const VideoMode *supportedModes = getSupportedModes();
    for (const UShort *videoModes = (const UShort *)(emu_main_mem + 
            infoBlock->VideoModePtr.address()); 
         *videoModes != 0xffff; videoModes++) {
        const struct ModeInfoBlock *const block = queryVideoMode(*videoModes);
        // Cannot query mode?
        if (!block) continue;
        // No hardware support?
        if (!(block->ModeAttributes & VIDEO_MODE_SUPPORTED)) continue;
        // Text mode?
        if (!(block->ModeAttributes & VIDEO_MODE_GRAPHIC)) continue;
        // No LFB support?
        if (!(block->ModeAttributes & VIDEO_MODE_SUPPORTS_LFB)) continue;
        ((ServerVideoMode *)supportedModes)[modeIndex].init(*videoModes, block);
        modeIndex++; NbSupportedModes++;
    }
    
    currentModeIndex = -1;
    return ERR_NONE;
}

status_t ServerScreen::cleanup() {
    return realModeEmu_cleanup();
}

/*
 * TODO should be turned into an I/O pager or something.
 */
static status_t
MapLFBPage(L4_Word_t physaddress, L4_Word_t spaceaddress)
{
    L4_KernelInterfacePage_t *kip;
    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();
    L4_ThreadId_t s0id = L4_GlobalId(L4_ThreadIdUserBase(kip), 1);
    L4_Msg_t msg;
    L4_Word_t reg[2];
    L4_Fpage_t page = L4_FpageLog2(physaddress, PAGE_BITS);
    L4_Fpage_t acceptedpage = L4_FpageLog2((L4_Word_t)spaceaddress, PAGE_BITS);
    reg[0] = page.raw;
    reg[1] = 0;
    L4_Put(&msg, MSG_SIGMA0, 2, reg, 0, 0);
    
    L4_Accept(L4_MapGrantItems(acceptedpage));
    status_t err = IpcCall(s0id, &msg, &msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    
    return err;
}

status_t
ServerScreen::setVideoMode(const UInt index) {
    const ServerVideoMode & mode = ((const ServerVideoMode *)getSupportedModes())[index];
    // Set the video mode using BIOS functions
    X86EMU_SETREG(AX, 0x4f02);
    X86EMU_SETREG(BX, mode.number() | VIDEO_MODE_USE_LFB);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        return ERR_UNKNOWN;
    }
    // Now map the frame buffer to our address space
    // We have to map the pages one by one - so let's first calculate how many pages
    // to map.
    L4_Word_t lfbPhysAddress = mode.lfbPhysicalAddress();
    UInt nbPages = mode.LFBNbPages();
    for (UInt i = 0; i < nbPages; i++) {
        status_t err = MapLFBPage(lfbPhysAddress + (PAGE_SIZE * i), 
                VBE_LFB_ADDRESS + (PAGE_SIZE * i));
        if (err != ERR_NONE) return err;
    }
    
    currentModeIndex = index;
    bytespp = getCurrentMode()->bytespp();
    xres = getCurrentMode()->xres();
    
    return ERR_NONE;
}
