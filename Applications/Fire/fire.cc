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
/// @file   Services/Devices/Vesa/fire.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// Implements the fire effect.
/// 

//$Id: fire.cc 349 2008-05-29 01:54:02Z hro $

#include <Debug.h>
#include <Random.h>
#include <Types.h>
#include <vesa/Screen.h>

extern Screen*      _screen;
extern VideoMode*   _vMode;

static const UShort RESISTANCE = 30;
static const UShort VIVACITY = 30;
static const UShort INTENSITY = 200;

void
fireRoot(UShort startx, UShort starty, UShort width, UShort height)
{
    ENTER;
    
    for (UShort j = starty; j < starty + height; j++) {
        // First light some new pixels
        for (int i = 0; i < VIVACITY; i++) {
            UShort pos = rand() % width;
            int color = INTENSITY + (rand() % (255 - INTENSITY));
            _screen->PutPixelRGB(startx + pos, j,
                                 RGBTriplet(color, color, color));
        }
        // Then shut other pixels down
        for (int i = 0; i < RESISTANCE; i++) {
            int pos = rand() % width;
            _screen->PutPixel(startx + pos, j, 0);
        }
    }
    EXIT;
}

void
fireEffect(UShort startx, UShort starty, UShort width, UShort height)
{
    for (int y = starty; y < starty + height; y++) {
        for (int x = startx; x < startx + width; x++) {
            UShort red, green, blue;
            
            // Lower pixel
            RGBTriplet pixel = _vMode->PixelToRGB(_screen->GetPixel(x, y + 1));
            red = pixel.red;
            green = pixel.green;
            blue = pixel.blue;
            
            // 2nd lower pixel
            pixel = _vMode->PixelToRGB(_screen->GetPixel(x, y + 2));
            red += pixel.red;
            green += pixel.green;
            blue += pixel.blue;
            
            // Lower left pixel
            if (x > startx) {
                pixel = _vMode->PixelToRGB(_screen->GetPixel(x - 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Lower right pixel
            if (x < startx + width - 1) {
                pixel = _vMode->PixelToRGB(_screen->GetPixel(x + 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Now fix the values
#define REDDEC 0
#define GREENDEC 5
#define BLUEDEC 100
            if (red > REDDEC) {
                red -= REDDEC; 
            }
            else {
                red = 0;
            }

            if (green > GREENDEC) {
                green -= GREENDEC;
            }
            else {
                green = 0;
            }

            if (blue > BLUEDEC) {
                blue -= BLUEDEC;
            }
            else {
                blue = 0;
            }

            red /= 4; green /= 4; blue /= 4;
            
            // And write the new pixel
            _screen->PutPixel(x, y, _vMode->RGBToPixel(RGBTriplet(red, green, blue)));
        }
    }
}

