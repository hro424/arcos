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

#ifndef VIDEOMODE_H_
#define VIDEOMODE_H_

#include <Types.h>
#include <l4/types.h>
#include "ModeInfoBlock.h"


struct RGBTriplet
{
    UByte   red;
    UByte   green;
    UByte   blue;

    RGBTriplet(UByte r, UByte g, UByte b) : red(r), green(g), blue(b) {}
};


class VideoMode {
private:
    UShort Number;
    UShort Xres;
    UShort Yres;
    UByte Bpp;
    
    UByte RedMaskSize;
    UByte RedFieldPos;
    UInt RedMask;
    UByte RedLoss;
    UByte GreenMaskSize;
    UByte GreenFieldPos;
    UInt GreenMask;
    UInt GreenLoss;
    UByte BlueMaskSize;
    UByte BlueFieldPos;
    UInt BlueMask;
    UInt BlueLoss;
    UByte NumberOfPages;
    
    L4_Word_t LFBAddress;

public:
    VideoMode(UShort number, const struct ModeInfoBlock *const info);
    
    UShort number() const { return Number; }
    UShort xres() const { return Xres; }
    UShort yres() const { return Yres; }
    UByte bpp() const { return Bpp; }
    UByte redMaskSize() const { return RedMaskSize; }
    UByte redFieldPos() const { return RedFieldPos; }
    UInt redMask() const { return RedMask; }
    UByte greenMaskSize() const { return GreenMaskSize; }
    UByte greenFieldPos() const { return GreenFieldPos; }
    UInt greenMask() const { return GreenMask; }
    UByte blueMaskSize() const { return BlueMaskSize; }
    UByte blueFieldPos() const { return BlueFieldPos; }
    UInt blueMask() const { return BlueMask; }
    UByte numberOfPages() const { return NumberOfPages; }
    L4_Word_t lfbPhysicalAddress() const { return LFBAddress; }
    
    const UInt rgbToPixel(const RGBTriplet rgb) const;
    const RGBTriplet pixelToRGB(const UInt pixel) const;
    
    void Print() const;
};

#endif /*VIDEOMODE_H_*/

