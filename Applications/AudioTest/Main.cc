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
/// @brief  Audio test
/// @file   Applications/AudioTest/Main.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

//$Id: Main.cc 410 2008-09-08 12:00:15Z hro $

#include <Debug.h>
#include <audio/Audio.h>
#include <System.h>

void callback(size_t size, UByte * buffer) {
    DOUT("SB Interrupt, size: %d\n", size);
    // First write the data for the next buffer
    static Short sample = 0;
    static bool dec = 0;
    static int sampleStep = 4;
    for (L4_Word_t i = 0; i < size; i++) {
        if (!dec) {
            sample += sampleStep;
            if (sample > 255) {
                sample = 255;
                dec = 1;
            }
        }
        else {
            sample -= sampleStep;
            if (sample < 0) {
                sample = 0;
                dec = 0;
            }
        }
        buffer[i] = sample;
    }
}

int
main(int argc, char* argv[])
{
    System.Print("In audio test!\n");

    if (argc < 2) {
        System.Print("Error. Audio server is not specified.\n");
        return 1;
    }

    System.Print("Connect to %s\n", argv[1]);

    AudioParameters params;
    params.rate = 22050;
    params.isSigned = false;
    params.is16Bits = false;
    params.isStereo = false;
    Audio::Initialize(argv[1], params, callback);
    Audio::Play();
    L4_Sleep(L4_TimePeriod(3000000));
    Audio::Stop();
    Audio::Shutdown();
    DOUT("Audio test finished\n");
    return ERR_NONE;
}

