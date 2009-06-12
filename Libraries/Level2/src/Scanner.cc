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
/// @brief  Keycode scanner
/// @file   Libraries/Level2/src/Scanner.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Scanner.cc 375 2008-08-08 07:53:30Z hro $

#include <Types.h>
#include "Scanner.h"

const char Scanner::keycode2ascii[] =
{
    0,
    27,      // ESC
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,      // L-CTRL
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,      // L-Shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,      // R-Shift
    '*',
    0,      // L-Alt
    ' ',    // Space
    0,      // CapsLock
    0,      // F1
    0,      // F2
    0,      // F3
    0,      // F4
    0,      // F5
    0,      // F6
    0,      // F7
    0,      // F8
    0,      // F9
    0,      // F10
    0,      // NumLock
    0,      // ScrLk
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3', '0', '.',
    0,      // SysReq
    0,      // Reserved
    0,      // Reserved
    0,      // F11
    0,      // F12
};

const char Scanner::keycode2ASCII[] =
{
    0,
    27,      // ESC
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,      // L-Ctrl
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,      // L-Shift
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0,      // R-Shift
    '*',
    0,      // L-Alt
    ' ',    // Space
    0,      // CapsLock
    0,      // F1
    0,      // F2
    0,      // F3
    0,      // F4
    0,      // F5
    0,      // F6
    0,      // F7
    0,      // F8
    0,      // F9
    0,      // F10
    0,      // NumLock
    0,      // ScrLk
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3', '0', '.',
    0,      // SysReq
    0,      // Reserved
    0,      // Reserved
    0,      // F11
    0,      // F12
};

//
//  PC/XT scancode format
//
//  |       8       | 4 | 2 | 1 | 8 | 4 | 2 | 1 |
//  | press/release |         keynumber         |
//

Scanner::Scanner() : k2a(keycode2ascii) {}

Bool
Scanner::IsPressed(UInt code)
{
    return (code & 0x80) == 0;
}

UInt
Scanner::Scan(UInt code)
{
    UInt keynumber = code & 0x7F;

    // Capture key presses
    if (IsPressed(code)) {
        switch (keynumber) {
            case L_SHIFT:
            case R_SHIFT:
                k2a = keycode2ASCII;
                return 0;
            case CTRL:
            case ALT:
                return 0;
            default:
                break;
        }

        return k2a[keynumber];
    }
    // Capture key releases
    else {
        if (code == 0xE0) {
            return 0;
        }

        switch (keynumber) {
            case L_SHIFT:
            case R_SHIFT:
                k2a = keycode2ascii;
                break;
            case CTRL:
            case ALT:
            default:
                break;
        }

        //return k2a[keynumber];
        return 0;
    }
}

