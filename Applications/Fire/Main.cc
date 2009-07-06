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
/// @brief  A fire frame screen saver.
/// @file   Applications/Fire/Main.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

//$Id: Main.cc 349 2008-05-29 01:54:02Z hro $

#include <Debug.h>
#include <Types.h>
#include <System.h>
#include <Random.h>
#include <vesa/Screen.h>
#include <vesa/VideoMode.h>

//#include "fire.h"

Screen*             _screen;
const VideoMode*    _vMode;

#define WIDTH 640
#define HEIGHT 480

#define PUTPIXEL(X,Y,COL) fb[(Y) * WIDTH + (X)] = (COL)
#define GETPIXEL(X,Y) fb[(Y) * WIDTH + (X)]

///// FIRE

#define RESISTANCE 50
#define VIVACITY 100
#define INTENSITY 200

void
MakeFireRoot(UShort *fb) {
    for (int j = HEIGHT - 2; j < HEIGHT; j++) {
        // First light some new pixels
        for (int i = 0; i < VIVACITY; i++) {
            int pos = rand() % WIDTH;
            int color = INTENSITY + (rand() % (255 - INTENSITY));
            PUTPIXEL(pos, j, _vMode->RGBToPixel(RGBTriplet(color, color, color)));
        }
        // Then shut other pixels down
        for (int i = 0; i < RESISTANCE; i++) {
            int pos = rand() % WIDTH;
            PUTPIXEL(pos, j, 0);
        }
    }
}

void
MakeFireEffect(UShort *fb) {
    for (int y = 400; y < HEIGHT - 2; y++)
        for (int x = 0; x < WIDTH; x++) {
            UShort red, green, blue;
            
            // Lower pixel
            RGBTriplet pixel = _vMode->PixelToRGB(GETPIXEL(x, y + 1));
            red = pixel.red;
            green = pixel.green;
            blue = pixel.blue;
            
            // 2nd lower pixel
            pixel = _vMode->PixelToRGB(GETPIXEL(x, y + 2));
            red += pixel.red;
            green += pixel.green;
            blue += pixel.blue;
            
            // Lower left pixel
            if (x > 0) {
                pixel = _vMode->PixelToRGB(GETPIXEL(x - 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Lower right pixel
            if (x < WIDTH - 1) {
                pixel = _vMode->PixelToRGB(GETPIXEL(x + 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Now fix the values
#define REDDEC 2
#define GREENDEC 10
#define BLUEDEC 150
            if (red > REDDEC) red -= REDDEC; else red = 0;
            if (green > GREENDEC) green -= GREENDEC; else green = 0;
            if (blue > BLUEDEC) blue -= BLUEDEC; else blue = 0;
            red /= 4; green /= 4; blue /= 4;
            
            // And write the new pixel
            PUTPIXEL(x, y, _vMode->RGBToPixel(RGBTriplet(red, green, blue)));
        }
}

///// FIRE


int
main()
{
    ENTER;

    addr_t fb;
    Screen screen;
    if (screen.Connect() != ERR_NONE) {
        return ERR_FATAL;
    }

    // Map the frame buffer.
    if (screen.SetVideoMode(640, 480, 16) != ERR_NONE) {
        System.Print(System.ERROR, "Cannot set video mode!\n");
        return ERR_NOT_FOUND;
    }
    
    if (screen.AllocateFrameBuffer() != ERR_NONE) {
        System.Print(System.ERROR, "Cannot allocate frame buffer!\n");
        return ERR_FATAL;
    }
    fb = screen.GetFrameBuffer();
    DOUT("frame buffer @ %p\n", fb);

    _screen = &screen;
    _vMode = screen.GetCurrentVideoMode();
    DOUT("current mode %lX\n", _vMode->Number);
    
    //UShort movingFirePos = 0;
    //UShort movingFireWidth = 50;
    //UByte movingFireDir = 1;
    for (int i = 0; i < 1000; i++) {
        MakeFireRoot((UShort*)fb);
        MakeFireEffect((UShort*)fb);
        /*
        fireRoot(0, _vMode->Yres - 2, _vMode->Xres, 2);
        fireRoot(movingFirePos, 120, movingFireWidth, 2);
        fireEffect(0, 0, _vMode->Xres, _vMode->Yres - 2);
        
        if (movingFireDir) {
            if (++movingFirePos >= _vMode->Xres - movingFireWidth) {
                movingFirePos = _vMode->Xres - movingFireWidth - 1;
                movingFireDir = 0;
            }
        }
        else {
            if (movingFirePos-- == 0) {
                movingFirePos = 0;
                movingFireDir = 1;
            }
        }
        */
    }

    screen.Disconnect();
    
    EXIT;
    return ERR_NONE;
}

