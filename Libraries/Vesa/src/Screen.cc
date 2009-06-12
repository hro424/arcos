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
/// @file   Libraries/Arc/Vesa/Screen.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  January 2008
/// 

//$Id: Screen.cc 386 2008-08-30 05:07:57Z hro $

#define SYS_DEBUG

#include <Debug.h>
#include <vesa/Screen.h>
#include <vesa/protocol.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <NameService.h>
#include <PageAllocator.h>
#include <System.h>

const char* const   Screen::SEVER_NAME = "vesa";
Screen*             Screen::_self;
L4_ThreadId_t       Screen::_server;

Screen *const
Screen::GetInstance()
{
    if (_self == 0) {
        _self = new Screen();
    }
    return _self;
}

stat_t
Screen::Connect()
{
    stat_t err;

    ENTER;

    //
    // Open a connection to VESA driver
    //
    err = NameService::Get(SERVER_NAME, &_tid);
    if (err != ERR_NONE) {
        return ERR_NOT_FOUND;
    }

    _session = new Session(_tid);
    if (_session == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    if (!_session->IsConnected()) {
        return ERR_NOT_FOUND;
    }

    _version = reg[1];
    _total_memory = reg[2];
    _bpp = reg[3];
    _xres = reg[4];
    _current_mode = reg[5];
    _num_modes = reg[6];

    //
    // VideoMode objects are shared through the session
    //
    ptr = reinterpret_cast<VideMode*>(_session->GetBaseAddress());

    //
    // Creates a list of the video modes
    //
    for (size_t i = 0; i < _num_modes; i++) {
        _vlist.Append(ptr);
        ptr++;
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Screen::Disconnect()
{
   if (_session != 0) {
        _vlist.Clear();
        delete _session;
    }
    return ERR_NONE;
}

stat_t
Screen::OpenFrameBuffer(const VideoMode& mode, addr_t* base)
{
    stat_t err;

    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

    if (_base != 0) {
        return;
    }
    //
    // Get the frame buffer (re)mapped to this side.
    //
    err = _session->Begin(reg, 1);
    if (err != ERR_NONE) {
        return err;
    }

    if (_fbpages < reg[0]) {
        if (_base != 0) {
            pfree(_base);
        }
        _fbpages = reg[0];
        _base = palloc(_fbpages);
    }

    fpage = L4_Fpage(_base, fbpages);
    L4_Accept(L4_MapGrantItems(fpage));
    err = _session->Get();
    L4_Accept(L4_MapGrantItems(L4_Nilpage));

    *base = _base;
    return ERR_NONE;
}

stat_t
Screen::OpenFrameBuffer(addr_t* base)
{

    return OpenFrameBuffer(mode, base);
}

stat_t
Screen::CloseFrameBuffer()
{
    if (_session == 0) {
        return ERR_NONE;
    }

    pfree(_base);
    _base = 0;

    //
    // Unmap the frame buffer
    //
    _session->End();

    return ERR_NONE;
}

stat_t
Screen::SetVideoMode(const VideoMode& mode)
{
    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

    _session->Put();

    return ERR_NONE;
}

stat_t
Screen::SetVideMode(UInt width, UInt height, UInt bpp)
{
    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

    SetVideoMode(mode);

    return ERR_NONE;
}

const VideoMode&
Screen::GetVideoMode()
{
    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

    return _vlist[_current_mode];
}

const List&
Screen::GetVideoModes()
{
    return _vlist;
}

UInt 
Screen::GetPixel(UShort x, UShort y)
{
    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

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
    if (_session == 0) {
        return ERR_DISCONNECTED;
    }

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

stat_t
Screen::SetVideoMode(UInt index) {
    L4_Msg_t    msg;
    L4_MapItem_t    item;
    //addr_t      src;
    //size_t      count;
    stat_t      err;
    ENTER;

    // Just make the call to the server, and wait to receive the
    // framebuffer...
    L4_Put(&msg, MSG_VESA_SET_VIDEO_MODE,
           1, static_cast<L4_Word_t*>(&index), 0, 0);
    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));
    err = Ipc::Call(_server, &msg, &msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    if (err != ERR_NONE) {
        return err;
    }

    L4_Get(&msg, 0, &item);
    DOUT("%.8lX:%.8lX\n", item.raw[0], item.raw[1]);
    _base = reinterpret_cast<UByte*>(L4_MapItemSndBase(item));

    /*
    err = Ipc::Call(_server, &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    src = L4_Get(&msg, 0);
    count = L4_Get(&msg, 1) >> PAGE_BITS;
    DOUT("src@%.8lX 0x%.8lX pages\n", src, count);
    _base = reinterpret_cast<UByte*>(palloc(count));
    err = Pager.Map(reinterpret_cast<addr_t>(_base), src, _server,
                    count, L4_ReadWriteOnly);
    */
    EXIT;
    return err;
}

stat_t
Screen::SetVideoMode(const VideoMode *const mode) {
    for (size_t i = 0; i < GetNbSupportedModes(); i++) {
        if (mode == &GetSupportedModes()[i]) {
            return SetVideoMode(i);
        }
    }
    return ERR_NOT_FOUND;
}

stat_t
Screen::SetVideoMode(UShort width, UShort height, UByte bpp) {
    const VideoMode *const supportedModes = GetSupportedModes();
    // Parse the list of modes until we find one that suits
    for (size_t i = 0; i < GetNbSupportedModes(); i++) {
        const VideoMode *candidate = &supportedModes[i];
        if (candidate->xres() == width && candidate->yres() == height &&
            candidate->bpp() == bpp) {
            return SetVideoMode(i);
        }
    }
    return ERR_NOT_FOUND;
}

void
Screen::printInfo() {
    System.Print(System.INFO,
                 "Board supports VBE %d.%d.\nVideo memory: %dKB\n", 
                 MajorVersion(), MinorVersion(), (int)TotalMemory() >> 12);
    System.Print(System.INFO, "Supported modes:\n");
    for (size_t i = 0; i < GetNbSupportedModes(); i++) {
        GetSupportedModes()[i].displayInfo();
    }
}

inline UInt
Screen::PixelOffset(UShort x, UShort y)
{
    return (y * _xres + x) * _bpp;
}

inline UInt
Screen::GetPixel8(UShort x, UShort y)
{
    return *(_base + PixelOffset(x, y));
}

inline UInt
Screen::GetPixel16(UShort x, UShort y)
{
    return *((UShort*)(_base + PixelOffset(x, y)));
}

inline UInt
Screen::GetPixel24(UShort x, UShort y)
{
    UInt offset = PixelOffset(x, y);
    return *((UShort*)(_base + offset)) + (*(_base + offset + 2)) << 16;
}

inline UInt
Screen::GetPixel32(UShort x, UShort y)
{
    return *((UInt*)(_base + PixelOffset(x, y)));
}

inline void
Screen::PutPixel8(UShort x, UShort y, UInt pixel)
{
    ENTER;
    *(_base + PixelOffset(x, y)) = pixel;
}

inline void
Screen::PutPixel16(UShort x, UShort y, UInt pixel)
{
    ENTER;
    *((UShort*)(_base + PixelOffset(x, y))) = pixel;
}

inline void
Screen::PutPixel24(UShort x, UShort y, UInt pixel)
{
    ENTER;
    UInt offset = PixelOffset(x, y);
    *((UShort*)(_base + offset)) = pixel & 0xffff;
    (*(_base + offset + 2)) = (pixel >> 16) & 0xff;
}

inline void
Screen::PutPixel32(UShort x, UShort y, UInt pixel)
{
    ENTER;
    *((UInt*)(_base + PixelOffset(x, y))) = pixel;
}

