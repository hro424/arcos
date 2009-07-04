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

struct RGBTriplet
{
    UByte   red;
    UByte   green;
    UByte   blue;

    RGBTriplet(UByte r, UByte g, UByte b) : red(r), green(g), blue(b) {}
};


struct VideoMode {
    UShort  Number;
    UShort  Xres;
    UShort  Yres;
    UByte   Bpp;
    UByte   Bytespp;
    
    UByte   RedMaskSize;
    UByte   RedFieldPos;
    UInt    RedMask;
    UByte   RedLoss;
    UByte   GreenMaskSize;
    UByte   GreenFieldPos;
    UInt    GreenMask;
    UInt    GreenLoss;
    UByte   BlueMaskSize;
    UByte   BlueFieldPos;
    UInt    BlueMask;
    UInt    BlueLoss;
    UByte   NumberOfPages;
    
    addr_t  LFBAddress;

    UInt RGBToPixel(RGBTriplet& rgb) const
    {
        UInt pixel = (rgb.red >> (8 - RedMaskSize)) << RedFieldPos;
        pixel |= (rgb.green >> (8 - GreenMaskSize)) << GreenFieldPos;
        pixel |= (rgb.blue >> (8 - BlueMaskSize)) << BlueFieldPos;
        return pixel;
    }

    RGBTriplet PixelToRGB(UInt pixel) const
    {
        UShort red, green, blue;
        red = ((pixel & RedMask) >> RedFieldPos) << (8 - RedMaskSize);
        green = ((pixel & GreenMask) >> GreenFieldPos) << (8 - GreenMaskSize);
        blue = ((pixel & BlueMask) >> BlueFieldPos) << (8 - BlueMaskSize);
        return RGBTriplet(red, green, blue);
    }
     
    void Print() const
    {
        System.Print("%x: %dx%dx%d, %d pages\n",
                     Number, Xres, Yres, Bpp, NumberOfPages);
    }

};

#endif /*VESA_VIDEOMODE_H_*/

