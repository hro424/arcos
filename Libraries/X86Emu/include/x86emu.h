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
/// @file   Include/x86emu.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

#ifndef X86EMU_H_
#define X86EMU_H_

#include <Types.h>
#include <x86emu/types.h>
#include <x86emu/regs.h>
#include <l4/types.h>

/// Size of the main memory, 1MB
#define X86EMU_MAIN_MEM_SIZE (0x1000 * 0x1000)
/// Number of pages that we will use to store code.
#define X86EMU_CODE_AREA_PAGES 4
/// Size of the code area. This limits the amount of code that can be executed.
#define X86EMU_CODE_AREA_SIZE (PAGE_SIZE * X86EMU_CODE_AREA_PAGES)
/// Start of the code area relative to the main memory
#define X86EMU_CODE_AREA_OFFSET PAGE_SIZE
/// Start of the user-data area relative to the main memory
#define X86EMU_DATA_AREA_OFFSET (X86EMU_CODE_AREA_OFFSET + X86EMU_CODE_AREA_SIZE)

/**
 * Initializes the x86 real-mode emulator.
 * 
 * @param ptr Pointer to 1MB of allocated memory that will be used
 * to simulate the real-mode memory. BIOS pages will be mapped to it,
 * so this pointer MUST be page-aligned.
 */
stat_t x86emu_init(addr_t ptr);

/**
 * Run the real-mode code between code_ptr_start and code_ptr_end, 
 * starting from position startpos.
 * The code will first be copied under the code area of the 
 * emulated real-mode memory before being executed. As such, it should
 * not be bigger than X86EMU_CODE_AREA_SIZE.
 */
stat_t x86emu_run(const L4_Word8_t *const code_ptr_start,
                  const L4_Word8_t *const code_ptr_end,
                  const L4_Word_t startpos);

/**
 * Cleanup all ressources allocated by the real-mode emulator and
 * makes the main memory pointer given to x86emu_init usuable once
 * again.
 */
void x86emu_cleanup(void);

#define X86EMU_SETREG(REG,VAL) M.x86.R_##REG = (VAL)
#define X86EMU_GETREG(REG) (M.x86.R_##REG)

/**
 * Use this macro to define some 16-bits real mode assembler code.
 * The code must NOT make any memory reference due to a limitation
 * in gas with 16-bits code. Fortunately, this is sufficient to call
 * interrupts.
 * @param NAME "name" of that code bit, for further reference with
 * REAL_MODE_ASM_BEGIN and REAL_MODE_ASM_END macros.
 * @param CODE 16-bits real mode assembler code.
 */
#define REAL_MODE_ASM(NAME, CODE) \
	asm volatile("\n" #NAME "_begin:\n\t" \
				 ".code16\n\t" \
				 CODE \
				 "\n.code32\n" \
				 #NAME "_end:")

/**
 * Put the beginning address of code bit named "X" into DEST (must be a pointer)
 */
#define REAL_MODE_ASM_BEGIN(X, DEST) asm volatile("movl $" #X "_begin, %0" : "=m"(DEST));

/**
 * Put the end address of code bit named "X" into DEST (must be a pointer)
 */
#define REAL_MODE_ASM_END(X, DEST) asm volatile("movl $" #X "_end, %0" : "=m"(DEST));

#endif /*X86EMU_H_*/
