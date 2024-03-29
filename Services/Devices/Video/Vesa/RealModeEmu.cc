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
/// @file   Services/Devices/Vesa/RealModeEmu.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Contains the data needed to real-mode emulation for the VESA driver.

#include <System.h>
#include <PageAllocator.h>
#include "RealModeEmu.h"

addr_t RME_BASE;

static const L4_Word8_t *codebegin, *codeend;

/**
 * The real-mode assembler that we will use to invoke int 0x10.
 */
void
__asmbits(void)
{
	REAL_MODE_ASM(int10h,
	             "int $0x10\n\t"
	             "hlt");
}

stat_t InitializeRealModeEmulator() {
    RME_BASE = palloc(X86EMU_MAIN_MEM_SIZE / PAGE_SIZE);
    if (x86emu_init(RME_BASE) != ERR_NONE) {
        pfree(RME_BASE, X86EMU_MAIN_MEM_SIZE / PAGE_SIZE);
        return ERR_FATAL;
    }
    REAL_MODE_ASM_BEGIN(int10h, codebegin);
    REAL_MODE_ASM_END(int10h, codeend);
    return ERR_NONE;
}

void CleanupRealModeEmulator() {
    pfree(RME_BASE, X86EMU_MAIN_MEM_SIZE / PAGE_SIZE);
    x86emu_cleanup();
}

void invokeInt10() {
    x86emu_run(codebegin, codeend, 0);
}

