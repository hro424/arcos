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
/// @since  2008

#ifndef VBEINFOBLOCK_H_
#define VBEINFOBLOCK_H_

#include <arc/types.h>
#include "RealModeAddress.h"

/**
 * The vbe info block, as we find it in memory.
 * See the VBE Core Functions Standard, version 3.0 for details.
 * This structure is only directly used internally.
 */
struct VbeInfoBlock {
    char    VbeSignature[4];
    UShort  VbeVersion;
    RealModeAddress   OemStringPtr;
    UByte   Capabilities[4];
    RealModeAddress   VideoModePtr;
    UShort  TotalMemory;
    UShort  OemSoftwareRev;
    RealModeAddress   OemVendorNamePtr;
    RealModeAddress   OemProductNamePtr;
    RealModeAddress   OemProductRevPtr;
    UByte   __reserved[222];
    UByte   OemData[256];
} __attribute__((packed));

#endif /*VBEINFOBLOCK_H_*/
