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
/// @file   Include/vesa/VideoMode.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///
/// Video mode definition.

#ifndef VESA_VIDEOMODE_H_
#define VESA_VIDEOMODE_H_

#include <System.h>
#include <Types.h>
#include <l4/types.h>
#include "RGBTriplet.h"

/**
 * Non-instanciable class which instances are mapped by the
 * Screen::Init() method, right after the Screen singleton.
 * 
 * This class describes all the useful characteristics of a
 * video mode.
 */
class VideoMode {
protected:
    UShort Number;
    UShort Xres;
    UShort Yres;
    UByte Bpp;
    UByte Bytespp;
    
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
    
    /**
     * Make this class non-instanciable
     */
    VideoMode();
    virtual ~VideoMode();
    
public:
    UShort number() const { return Number; }
    UShort xres() const { return Xres; }
    UShort yres() const { return Yres; }
    UByte bpp() const { return Bpp; }
    UByte bytespp() const { return Bytespp; }
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
    
    const UInt RGBToPixel(const RGBTriplet rgb) const;
    const RGBTriplet PixelToRGB(const UInt pixel) const;
     
    void displayInfo() const;
};

inline const UInt
VideoMode::RGBToPixel(const RGBTriplet rgb) const
{
    UInt pixel = (rgb.red() >> (8 - redMaskSize())) << redFieldPos();
    pixel |= (rgb.green() >> (8 - greenMaskSize())) << greenFieldPos();
    pixel |= (rgb.blue() >> (8 - blueMaskSize())) << blueFieldPos();
    return pixel;
}

inline const RGBTriplet
VideoMode::PixelToRGB(const UInt pixel) const
{
    UShort red, green, blue;
    red = ((pixel & redMask()) >> (redFieldPos())) << (8 - redMaskSize());
    green = ((pixel & greenMask()) >> (greenFieldPos())) << (8 - greenMaskSize());
    blue = ((pixel & blueMask()) >> (blueFieldPos())) << (8 - blueMaskSize());
    return RGBTriplet(red, green, blue);
}

inline void
VideoMode::displayInfo() const
{
    System.Print(System.INFO, "%x: %dx%dx%d, %d pages\n",
                 Number, Xres, Yres, Bpp, NumberOfPages);
}

#endif /*VESA_VIDEOMODE_H_*/

