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
#include <vesa/Screen.h>

#include "fire.h"

int
main()
{
    ENTER;

    // First initialize the VESA driver
    // Frame buffer is not mapped yet at this moment.
    if (Screen::Initialize() != ERR_NONE) {
        return 1;
    }

    Screen* const screen = Screen::GetInstance();
    // Map the frame buffer.
    if (screen->SetVideoMode(640, 480, 16) != ERR_NONE) {
        System.Print(System.ERROR, "Cannot set video mode!\n");
        screen->printInfo();
        return ERR_NOT_FOUND;
    }
    
    DOUT("frame buffer @ %p\n", screen->GetFrameBuffer());
    const VideoMode* const vMode = screen->GetCurrentMode();
    vMode->displayInfo();
    
    UShort movingFirePos = 0;
    UShort movingFireWidth = 50;
    UByte movingFireDir = 1;
    while (1) {
        DOUT("\n");
        fireRoot(0, vMode->yres() - 2, vMode->xres(), 2);
        DOUT("\n");
        fireRoot(movingFirePos, 120, movingFireWidth, 2);
        DOUT("\n");
        fireEffect(0, 0, vMode->xres(), vMode->yres() - 2);
        DOUT("\n");
        
        if (movingFireDir) {
            if (++movingFirePos >= vMode->xres() - movingFireWidth) {
                movingFirePos = vMode->xres() - movingFireWidth - 1;
                movingFireDir = 0;
            }
        }
        else {
            if (movingFirePos-- == 0) {
                movingFirePos = 0;
                movingFireDir = 1;
            }
        }
    }
    
    EXIT;
    return ERR_NONE;
}

