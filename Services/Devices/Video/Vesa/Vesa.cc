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
/// @file   Services/Devices/Vesa/Vesa.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

#include <Debug.h>
#include <System.h>
#include <Random.h>

#include "Vbe.h"
#include "VideoMode.h"

#define WIDTH 640
#define HEIGHT 480

#define PUTPIXEL(X,Y,COL) fb[(Y) * WIDTH + (X)] = (COL)
#define GETPIXEL(X,Y) fb[(Y) * WIDTH + (X)]

const VideoMode *vMode;

UShort*
GetFrameBuffer() {
    return (UShort*)VBE_LFB_ADDRESS;
}

///// FIRE

#define RESISTANCE 50
#define VIVACITY 100
#define INTENSITY 200

void
fireRoot()
{
    UShort* fb = GetFrameBuffer();
    for (int j = HEIGHT - 2; j < HEIGHT; j++) {
        // First light some new pixels
        for (int i = 0; i < VIVACITY; i++) {
            int pos = rand() % WIDTH;
            int color = INTENSITY + (rand() % (255 - INTENSITY));
            PUTPIXEL(pos, j, vMode->rgbToPixel(RGBTriplet(color, color, color)));
        }
        // Then shut other pixels down
        for (int i = 0; i < RESISTANCE; i++) {
            int pos = rand() % WIDTH;
            PUTPIXEL(pos, j, 0);
        }
    }
}

void
fireEffect()
{
    UShort* fb = GetFrameBuffer();
    for (int y = 400; y < HEIGHT - 2; y++) {
        for (int x = 0; x < WIDTH; x++) {
            UShort red, green, blue;
            
            // Lower pixel
            RGBTriplet pixel = vMode->pixelToRGB(GETPIXEL(x, y + 1));
            red = pixel.red;
            green = pixel.green;
            blue = pixel.blue;
            
            // 2nd lower pixel
            pixel = vMode->pixelToRGB(GETPIXEL(x, y + 2));
            red += pixel.red;
            green += pixel.green;
            blue += pixel.blue;
            
            // Lower left pixel
            if (x > 0) {
                pixel = vMode->pixelToRGB(GETPIXEL(x - 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Lower right pixel
            if (x < WIDTH - 1) {
                pixel = vMode->pixelToRGB(GETPIXEL(x + 1, y + 1));
                red += pixel.red;
                green += pixel.green;
                blue += pixel.blue;
            }
            
            // Now fix the values
#define REDDEC 2
#define GREENDEC 10
#define BLUEDEC 150
            if (red > REDDEC) {
                red -= REDDEC; else red = 0;
            }
            if (green > GREENDEC) {
                green -= GREENDEC; else green = 0;
            }
            if (blue > BLUEDEC) {
                blue -= BLUEDEC; else blue = 0;
            }
            red /= 4; green /= 4; blue /= 4;
            
            // And write the new pixel
            PUTPIXEL(x, y, vMode->rgbToPixel(RGBTriplet(red, green, blue)));
        }
    }
}

///// FIRE


int
server_main(int argc, char* argv[], int stat)
{
    addr_t      base;
    Vbe         vbe;

    System.Print("VBE driver starting...\n");
    base = InitializeRealModeEmulator();
    if (base == 0) {
        return -1;
    }

    vbe.Initialize();

    DOUT("Set video mode\n");
    if (vbe.SetVideoMode(2) != ERR_NONE) {
        System.Print("Cannot set video mode!\n");
        return ERR_UNKNOWN;
    }

    vMode = vbe.GetVideoMode(2);

    for (;;) {
        fireRoot();
        fireEffect();
    }

    vbe.Finalize();

    // TODO: Enable that when x86emu unmaps the bios pages on cleanup.
    // pfree(main_mem);
    CleanupRealModeEmulator();
	System.Print("VBE driver exiting...\n");
	return 0;
}

