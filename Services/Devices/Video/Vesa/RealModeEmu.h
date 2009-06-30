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
/// @file   Services/Devices/Vesa/RealModeEmu.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Contains the data needed to real-mode emulation for the VESA driver.

#ifndef REALMODEEMU_H_
#define REALMODEEMU_H_

#include "x86emu.h"

/**
 * Pointer to the real-mode emulated memory.
 */
extern char *emu_main_mem;

/*
 * These macros interpret the return of VBE functions.
 */
#define VESA_VBE_FUNCTION_SUPPORTED(X) ((X & 0xff) == 0x4f)
#define VESA_VBE_FUNCTION_SUCCESSFUL(X) (((X & 0xff00) >> 8) == 0)
#define VESA_VBE_SUCCESS(X) (VESA_VBE_FUNCTION_SUPPORTED(X) && VESA_VBE_FUNCTION_SUCCESSFUL(X))

/**
 * Initializes the x86 real-mode emulation layer. Must be called before
 * anything!
 */
status_t realModeEmu_init();

/**
 * Cleanup the x86 real-mode emulation layer.
 */
status_t realModeEmu_cleanup();

/**
 * Invoke interrupt 0x10. Intput and output are handled
 * through the virtual x86 registers of x86emu.
 */
void invokeInt10();

#endif /*REALMODEEMU_H_*/
