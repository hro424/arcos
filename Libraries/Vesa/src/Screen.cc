/*
 *
 *  Copyright (C) 2008-2009, Waseda University.
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
/// @file   Libraries/Arc/Vesa/Screen.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  January 2008
/// 


#define SYS_DEBUG

#include <Debug.h>
#include <vesa/Screen.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <NameService.h>
#include <PageAllocator.h>
#include <String.h>
#include <System.h>

const char* Screen::SERVER_NAME = "vesa";

stat_t
Screen::Connect()
{
    stat_t      err;
    L4_Word_t   reg[3];

    ENTER;

    //
    // Open a connection to VESA driver
    //
    err = NameService::Get(SERVER_NAME, &_server);
    if (err != ERR_NONE) {
        return ERR_NOT_FOUND;
    }

    _session = new Session();
    if (_session == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    _session->Connect(_server, reg, 3);

    if (!_session->IsConnected()) {
        return ERR_NOT_FOUND;
    }

    _version = reg[0];
    _total_memory = reg[1];
    _num_modes = reg[2];

    //
    // VideoMode objects are shared through the session
    //
    VideoMode* ptr = reinterpret_cast<VideoMode*>(_session->GetBaseAddress());

    //
    // Creates a list of the video modes
    //
    for (size_t i = 0; i < _num_modes; i++) {
        memcpy(&_vlist[i], &ptr[i], sizeof(VideoMode));
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Screen::AllocateFrameBuffer(const VideoMode* mode)
{
    stat_t err;

    if (mode == 0) {
        return ERR_NOT_FOUND;
    }

    if (_fb_base != 0) {
        return ERR_BUSY;
    }

    _fb_pages = mode->NumberOfPages;
    _fb_base = palloc_shm(_fb_pages, L4_Myself(), L4_ReadWriteOnly);
    if (_fb_base == 0) {
        _fb_pages = 0;
        return ERR_OUT_OF_MEMORY;
    }

    err = Pager.Map(_fb_base, L4_ReadWriteOnly,
                    mode->LFBAddress, L4_nilthread);
    if (err != ERR_NONE) {
        pfree(_fb_base, _fb_pages);
        _fb_base = 0;
        _fb_pages = 0;
        return err;
    }

    return ERR_NONE;
}

stat_t
Screen::AllocateFrameBuffer()
{
    return AllocateFrameBuffer(_current_mode);
}

void
Screen::ReleaseFrameBuffer()
{
    if (_fb_base != 0) {
        pfree(_fb_base, _fb_pages);
        _fb_base = 0;
        _fb_pages = 0;
    }
}



UInt 
Screen::GetPixel(UShort x, UShort y)
{
    switch (_bpp) {
        case 1:
            return GetPixel8(x, y);
        case 2:
            return GetPixel16(x, y);
        case 3:
            return GetPixel24(x, y);
        case 4:
            return GetPixel32(x, y);
        default:
            return ERR_NOT_FOUND;
    }
}
    
void 
Screen::PutPixel(UShort x, UShort y, UInt pixel)
{ 
    switch (_bpp) {
        case 1:
            PutPixel8(x, y, pixel); return;
        case 2:
            PutPixel16(x, y, pixel); return;
        case 3:
            PutPixel24(x, y, pixel); return;
        case 4:
            PutPixel32(x, y, pixel); return;
    }
    EXIT;
}

