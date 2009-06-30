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
/// @file   Services/Devices/Vesa/ModeInfoBlock.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Defines the ModeInfoBlock VESA VBE 2.0 structure.

#ifndef MODEINFOBLOCK_H_
#define MODEINFOBLOCK_H_

#include <arc/types.h>

/*
 * Masks for the ModesAttributes field of
 * ModeInfoBlock.
 */
#define VIDEO_MODE_SUPPORTED (1 << 0)
#define VIDEO_MODE_GRAPHIC (1 << 4)
#define VIDEO_MODE_SUPPORTS_LFB (1 << 7)

/*
 * Masks for the 0x4f02 (set video mode) function
 */
#define VIDEO_MODE_USE_LFB (1 << 14)
/**
 * If set when calling setVideoMode, the display memory is not
 * erased. May be useful for driver recovery.
 */
#define VIDEO_MODE_KEEP_DISPLAY_MEMORY (1 << 15)

/**
 * Contains information about a specific video mode
 */
struct ModeInfoBlock {
    // Historical VBE information
    UShort  ModeAttributes;
    UByte   WinAAttributes;
    UByte   WinBAttributes;
    UShort   WinGranularity;
    UShort   WinSize;
    UShort   WinASegment;
    UShort   WinBSegment;
    UInt   WinFuncPtr;
    UShort   BytesPerScanLine;
    
    // VBE 1.2 information
    UShort  XResolution;
    UShort  YResolution;
    UByte   XCharSize;
    UByte   YCharSize;
    UByte   NumberOfPlanes;
    UByte   BitsPerPixel;
    UByte   NumberOfBanks;
    UByte   MemoryModel;
    UByte   BankSize;
    UByte   NumberOfImagePages;
    UByte   __reserved;
    
    // Direct color fields
    UByte   RedMaskSize;
    UByte   RedFieldPosition;
    UByte   GreenMaskSize;
    UByte   GreenFieldPosition;
    UByte   BlueMaskSize;
    UByte   BlueFieldPosition;
    UByte   RsvdMaskSize;
    UByte   RsvdFieldPosition;
    UByte   DirectColorModeInfo;
    
    // VBE 2.0 information
    UInt   PhysBasePtr;
    UInt   __reserved2;
    UShort  __reserved3;
    
} __attribute__((packed));

#endif /*MODEINFOBLOCK_H_*/
