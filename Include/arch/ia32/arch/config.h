/*
 *
 *  Copyright (C) 2008, Waseda University. All rights reserved.
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
/// @brief  System configuration on IA32 architecture
/// @file   Include/arch/ia32/config.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
/// @see    $(L4SRC)/kernel/src/glue/v4-x86/x32/config.h
///

//$Id: config.h 393 2008-09-02 11:49:52Z hro $

#ifndef ARC_IA32_CONFIG_H
#define ARC_IA32_CONFIG_H

struct PhysLayout
{
    static const addr_t USER_AREA_START =   0x00000000UL;
    static const addr_t DOS_AREA_START =    0x00000000UL;
    static const addr_t DOS_AREA_END =      0x00010000UL;

    static const addr_t VIDEO_AREA_START =  0x000A0000UL;
    static const addr_t VIDEO_AREA_END =    0x000C0000UL;
    static const addr_t EXBIOS_AREA_START = 0x000E0000UL;
    static const addr_t EXBIOS_AREA_END =   0x000F0000UL;
    static const addr_t BIOS_AREA_START =   0x000F0000UL;
    static const addr_t BIOS_AREA_END =     0x00100000UL;
    // The region from the end of RAM up to ROOT_UTCB_START is used to
    // map I/O (e.g. frame buffer).
    static const addr_t ROOT_UTCB_START =   0xBF000000UL;
    static const addr_t USER_AREA_END =     0xC0000000UL;

    static const addr_t SIGMA0_START =      0x00090000UL;
    static const addr_t ROOTTASK_START =    0x00300000UL;
};

struct VirtLayout
{
    static const addr_t KIP_START =         0x07000000UL;
    static const addr_t P3_HEAP_START =     0x08080000UL;
    static const addr_t P3_HEAP_END =       0x0A080000UL;
    static const addr_t P3_STACK_END =      0x0B000000UL;
    static const addr_t SHM_START =         0x18000000UL;
    static const addr_t USER_TEXT_START =   0x20000000UL;
    static const addr_t USER_STACK_END =    0xC0000000UL;
};

#define PAGE_BITS   12UL
#define PAGE_SIZE   (1UL << PAGE_BITS)
#define PAGE_MASK   ~(PAGE_SIZE - 1UL)

#define PAGE_ALIGN(addr)            (((addr) + PAGE_SIZE - 1UL) & PAGE_MASK)

#endif // ARC_IA32_CONFIG_H

