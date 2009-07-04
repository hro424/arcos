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
/// @file   Services/Devices/Vesa/VbeInfoBlock.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Defines the VbeInfoBlock VESA VBE 2.0 structure.
///

#ifndef VBEINFOBLOCK_H_
#define VBEINFOBLOCK_H_

#include <Types.h>
#include <System.h>
#include "RealModeAddress.h"

/**
 * Address to which we want to map the linear frame buffer.
 */
//#define IOMAP_START     0x10000000UL
//#define VBE_LFB_ADDRESS IOMAP_START

/**
 * The vbe info block, as we find it in memory.
 * See the VBE Core Functions Standard, version 3.0 for details.
 * This structure is only directly used internally.
 */
/**
 * Contains information about the global capabalities of the graphic board.
 */
struct VbeInfoBlock {
    char                vbe_signature[4];
    UShort              vbe_version;
    RealModeAddress     oem_string_ptr;
    UByte               capabilities[4];
    RealModeAddress     video_mode_ptr;
    UShort              total_memory;
    UShort              oem_software_rev;
    RealModeAddress     oem_vendor_name_ptr;
    RealModeAddress     oem_product_name_ptr;
    RealModeAddress     oem_producet_rev_ptr;
    UByte               __reserved[222];
    UByte               oem_data[256];

    /**
     * Returns the 4 characters array that makes the VBE signature.
     * This is not a string and is not null-terminated!
     */
    const char* GetSignature() const { return vbe_signature; }

    UByte GetMajorVersion() const { return vbe_version >> 8; }

    UByte GetMinorVersion() const { return vbe_version & 0xff; }

    const char* GetOemString() const
    { return (const char*)oem_string_ptr.address(); }

    const UShort* GetModeList() const
    { return (const UShort*)video_mode_ptr.address(); }

    UInt GetTotalMemory() const { return total_memory << 16; }

    void Print() const {
        System.Print("Board is %s, supports VBE %d.%d.\nVideo memory: %dKB\n",
                     GetOemString(), GetMajorVersion(), GetMinorVersion(),
                     GetTotalMemory() >> 12);
    }

} __attribute__((packed));

#endif /*VBEINFOBLOCK_H_*/

