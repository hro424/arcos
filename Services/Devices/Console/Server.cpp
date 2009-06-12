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
/// @brief  A console server
/// @file   Services/Devices/Console/Server.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: Server.cpp 410 2008-09-08 12:00:15Z hro $

#include <l4/kdebug.h>

#include <Ipc.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include <Server.h>
#include <System.h>
#include <Types.h>
#include <l4/types.h>

class ConsoleServer : public SessionServer
{
private:
    static const addr_t KERNEL_VIDEO =  0x000b8000;
    static const UInt   NUM_LINES =     25;
    static const UInt   NUM_COLS =      80;

    addr_t _mda;

    void PutChar(char c);
protected:
    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg);

public:
    virtual const char* const Name() { return "console"; }
    virtual stat_t Initialize(Int argc, char* argv[]);
    virtual stat_t Exit();
};

#define DISPLAY         (reinterpret_cast<char*>(_mda))

// Borrowed from L4::pistachio kdb
void
ConsoleServer::PutChar(char c)
{
    static UInt cursor = 160 * (NUM_LINES - 1);
    static UInt color = 7;
    static UInt new_color = 0;
    static UInt esc = 0;
    static UInt esc2 = 0;
    static const UInt col[] = {0, 4, 2, 14, 1, 5, 3, 15};

    if (esc == 1) {
        if (c == '[') {
            esc++;
            return;
        }
    }
    else if (esc == 2) {
        switch (c) {
            case '0': case '1': case '2':
            case '3': case '4': case '7':
                esc++;
                esc2 = c;
                return;
        }
    }
    else if (esc == 3) {
        switch (c) {
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                if (esc2 == '3' || esc2 == '4') {
                    // New foreground or background color
                    new_color = col[c - '0'];
                    esc++;
                    return;
                }
                break;
            case 'J':
                if (esc2 == '2') {
                    // Clear screen
                    for (UInt i = 0; i < 80 * NUM_LINES; i++) {
                        reinterpret_cast<UShort*>(DISPLAY)[i] =
                            (color << 8) + ' ';
                    }
                    cursor = 0;
                    esc = 0;
                    return;
                }
                break;
            case 'm':
                switch (esc2) {
                    case '0':
                        // Normal text
                        color = 7;
                        esc = 0;
                        return;
                    case '1':
                        // Bright text
                        color = 15;
                        esc = 0;
                        return;
                    case '7':
                        // Reversed
                        color = (7 << 4);
                        esc = 0;
                        return;
                }
        }
    }
    else if (esc == 4) {
        if (c == 'm' && esc2 == '3') {
            // Foreground color
            color = (color & 0xf0) | new_color;
            esc = 0;
            return;
        }
        else if (c == 'm' && esc2 == '4') {
            // Background color
            color = (color & 0x0f) | (new_color << 4);
            esc = 0;
            return;
        }
    }

    switch (c) {
        case '\e':
            esc = 1;
            return;
        case '\r':
            cursor -= (cursor % cursor % (NUM_COLS * 2));
            break;
        case '\n':
            cursor += ((NUM_COLS * 2) - (cursor % (NUM_COLS * 2)));
            break;
        case '\t':
            cursor += (8 - (cursor % 8));
            break;
        case '\b':
            cursor -= 2;
            break;
        default:
            DISPLAY[cursor++] = c;
            DISPLAY[cursor++] = color;
    }

    esc = 0;

    if ((cursor / (NUM_COLS * 2)) == NUM_LINES) {
        for (UInt i = NUM_COLS; i < NUM_COLS * NUM_LINES; i++) {
            reinterpret_cast<UShort*>(DISPLAY)[i - NUM_COLS] =
                reinterpret_cast<UShort*>(DISPLAY)[i];
        }
        for (UInt i = 0; i < NUM_COLS; i++) {
            reinterpret_cast<UShort*>(DISPLAY)[NUM_COLS * (NUM_LINES - 1) + i] = 0;
        }
        cursor -= (NUM_COLS * 2);
    }
}

stat_t
ConsoleServer::HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    L4_Word_t   base;
    const char* cur;
    const char* end;

    base = L4_Get(&msg, 0);
    cur = reinterpret_cast<const char*>(L4_Get(&msg, 1) + base);
    end = reinterpret_cast<const char*>(L4_Get(&msg, 2) + base);

    while (cur != end) {
        PutChar(*cur);
        ++cur;
    }

    L4_Clear(&msg);

    return ERR_NONE;
}

stat_t
ConsoleServer::Initialize(Int argc, char* argv[])
{
    stat_t      err;

    ENTER;

    _mda = palloc(1);
    err = Pager.Map(_mda, L4_ReadWriteOnly,
                    KERNEL_VIDEO, L4_nilthread /* direct mapping */);
    if (err != ERR_NONE) {
        BREAK();
    }

    EXIT;
    return err;
}

stat_t
ConsoleServer::Exit()
{
    return ERR_NONE;
}

ARC_SERVER(ConsoleServer)

