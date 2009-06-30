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
/// @file   Services/Devices/Vesa/VideoMode.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Video mode definition.

#include <arc/console.h>
#include <arc/system.h>
#include "ServerVideoMode.h"
#include "ModeInfoBlock.h"

void ServerVideoMode::init(UShort nb, const ModeInfoBlock *info)
{
    Number = nb;
    Xres = info->XResolution;
    Yres = info->YResolution;
    Bpp = info->BitsPerPixel;
    RedMaskSize = info->RedMaskSize;
    RedFieldPos = info->RedFieldPosition;
    GreenMaskSize = info->GreenMaskSize;
    GreenMaskSize = info->GreenMaskSize;
    BlueFieldPos = info->BlueFieldPosition;
    BlueFieldPos = info->BlueFieldPosition;
    NumberOfPages = info->NumberOfImagePages + 1;
    LFBAddress = info->PhysBasePtr;
    
    switch (info->BitsPerPixel) {
    case 15:
        Bytespp = 2;
        break;
    default:
        Bytespp = info->BitsPerPixel / 8;
        break;
    }
    RedMask = 0;
    for (int i = 0; i < RedMaskSize; i++) RedMask |= (1 << RedFieldPos + i);
    RedLoss = 0xff >> (8 - RedMaskSize) << (8 - RedMaskSize);
    GreenMask = 0;
    for (int i = 0; i < GreenMaskSize; i++) GreenMask |= (1 << GreenFieldPos + i);
    GreenLoss = 0xff >> (8 - GreenMaskSize) << (8 - GreenMaskSize);
    BlueMask = 0;
    for (int i = 0; i < BlueMaskSize; i++) BlueMask |= (1 << BlueFieldPos + i);
    BlueLoss = 0xff >> (8 - BlueMaskSize) << (8 - BlueMaskSize);
}

UInt ServerVideoMode::LFBSize() const {
    return xres() * yres() * bytespp() * numberOfPages();
}

UInt ServerVideoMode::LFBNbPages() const {
    return (LFBSize() + PAGE_SIZE - 1) / PAGE_SIZE;
}
