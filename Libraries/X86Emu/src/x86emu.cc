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
/// @file   Libraries/X86emu/x86emu.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

#include <x86emu.h>
#include <x86emu/x86emu.h>
#include <x86emu/arcstubs.h>
#include <Ipc.h>
#include <System.h>
#include <String.h>

/*
 * Our memory model is that we allocate one megabyte of memory statically for
 * simulating the real-mode memory. We can then map BIOS areas there so that the
 * emulator finds them as expected.
 */
/// Pointer to the emulated memory
L4_Word8_t *main_mem = 0;
/// Pointer to the code area.
static L4_Word8_t *code_area = 0;


/**
 * Uses system IPC to map a bios page into the address space.
 * TODO: move into the right library...
 */
static stat_t
MapBiosPage(L4_Word_t biosaddress, L4_Word_t mapaddress)
{
    L4_Msg_t msg;
    L4_Word_t reg[2];
    stat_t err;
	L4_Fpage_t fpage = L4_FpageLog2(mapaddress, PAGE_BITS);
	
	L4_Accept(L4_MapGrantItems(fpage));
	reg[0] = biosaddress;
	reg[1] = (L4_Word_t) mapaddress;
	L4_MsgPut(&msg, MSG_PAGER_MAPBIOSPAGE, 2, reg, 0, (void *) 0);
	err = Ipc::Call(L4_Pager(), &msg, &msg);
	L4_Accept(L4_MapGrantItems(L4_Nilpage));
    return err;
}

stat_t
x86emu_init(addr_t ptr)
{
    // Reset all the emulation environment
    memset(&M, 0, sizeof(M));
    
	main_mem = (L4_Word8_t *)ptr;
	// Code area can be located right after the first page, that
	// we are going to map from the bios.
	code_area = main_mem + PAGE_SIZE;
	
	// Setup x86emu callbacks
    X86EMU_setupPioFuncs(&my_pioFuncs);
	X86EMU_setupMemFuncs(&my_memFuncs);
	// Must x86emu setup
	M.mem_base = (long unsigned int) main_mem;
	M.mem_size = X86EMU_MAIN_MEM_SIZE;
	// M.x86.debug = DEBUG_DECODE_F;
	
	// Map interrupts table
    stat_t err = MapBiosPage(0x0000, (L4_Word_t)main_mem);
    if (err != ERR_NONE) return err;
        
    // Map BIOS-reserved memory area
    for (L4_Word_t address = 0x9f000; address < 0x100000; address += PAGE_SIZE) {
        err = MapBiosPage(address, (L4_Word_t)(main_mem + address));
        if (err != ERR_NONE) return err;
    }
    
	return ERR_NONE;
}

/**
 *  Cleanup ressources allocated by the real-mode x86 emulator.
 */
void
x86emu_cleanup(void)
{
	main_mem = 0;
	code_area = 0;
	/* TODO: Unmap previously mapped pages! */
}

stat_t
x86emu_run(const L4_Word8_t *const code_ptr_start, 
           const L4_Word8_t *const code_ptr_end, const L4_Word_t startpos)
{
	memcpy(code_area, code_ptr_start, code_ptr_end - code_ptr_start);
	
	M.x86.R_SS = code_area - main_mem + X86EMU_CODE_AREA_SIZE >> 4;
	M.x86.R_SP = PAGE_SIZE;
	M.x86.R_CS = code_area - main_mem >> 4;
	M.x86.R_IP = startpos;
	X86EMU_exec();
    return ERR_NONE;
}
