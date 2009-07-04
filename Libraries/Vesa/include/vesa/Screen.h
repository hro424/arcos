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
/// @file   Include/vesa/Screen.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

//$Id: Screen.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_VESASCREEN_H_
#define ARC_VESASCREEN_H_

#include <Debug.h>
#include <Types.h>
#include <Session.h>
#include <vesa/VideoMode.h>
#include <sys/Config.h>

///
/// Controls the physical screen, provides routines to draw on it or to get
/// the framebuffer for direct access.
///

class Screen
{
protected:    
    static const char*  SERVER_NAME;

    L4_ThreadId_t       _server;

    Session*            _session;

    ///
    /// VBE version
    ///
    UShort              _version;

    ///
    /// Size of video memory
    ///
    UInt                _total_memory;

    /*
     * Cache the number of bytes per pixel and x resolution for the current
     * mode - this makes offsets calculation faster.
     */
    UByte   _bpp;
    UShort  _xres;

    static const UInt   MAX_SUPPORTED_MODES = 64;

    VideoMode           _vlist[MAX_SUPPORTED_MODES];
    
    VideoMode*          _current_mode;
    UInt                _num_modes;

    ///
    /// Base address of the frame buffer (client side)
    ///
    addr_t              _fb_base;
    UInt                _fb_pages;

    /**
     * Gives the offset (in bytes) to reach pixel (x,y)
     * from the framebuffer.
     */
    UInt PixelOffset(UShort x, UShort y)
    { return (y * _current_mode->Xres + x) * _current_mode->Bpp; }

    UByte GetPixel8(UShort x, UShort y)
    { return *(UByte*)(_fb_base + PixelOffset(x, y)); }
    
    UShort GetPixel16(UShort x, UShort y)
    { return *(UShort*)(_fb_base + PixelOffset(x, y)); }

    UInt GetPixel24(UShort x, UShort y)
    {
        UInt offset = PixelOffset(x, y);
        return (UInt)*((UShort*)(_fb_base + offset)) +
            ((*(UByte*)(_fb_base + offset + 2)) << 16);
    }

    UInt GetPixel32(UShort x, UShort y)
    { return *((UInt*)(_fb_base + PixelOffset(x, y))); }

    void PutPixel8(UShort x, UShort y, UByte pixel)
    { *(UByte*)(_fb_base + PixelOffset(x, y)) = pixel; }

    void PutPixel16(UShort x, UShort y, UShort pixel)
    { *(UShort*)(_fb_base + PixelOffset(x, y)) = pixel; }

    void PutPixel24(UShort x, UShort y, UInt pixel)
    {
        UInt offset = PixelOffset(x, y);
        *(UShort*)(_fb_base + offset) = pixel & 0xffff;
        *(UByte*)(_fb_base + offset + 2) = (pixel >> 16) & 0xff;
    }

    void PutPixel32(UShort x, UShort y, UInt pixel)
    { *((UInt*)(_fb_base + PixelOffset(x, y))) = pixel; }
   
public:
    Screen() : _fb_base(0), _fb_pages(0) {}

    virtual ~Screen() {}
    
    stat_t Connect();

    stat_t Disconnect()
    {
        if (_session != 0) {
            delete _session;
        }
        return ERR_NONE;
    }

    /**
     * Vesa major version
     */
    UByte GetMajorVersion() { return _version >> 8; }

    /**
     * Vesa minor version
     */
    UByte GetMinorVersion() { return _version & 0xff; }

    ///
    /// Obtains the total amount of video memory in bytes.
    ///
    UInt GetTotalMemory() { return _total_memory << 16; }

    size_t GetNumberOfSupportedModes() { return _num_modes; }

    const VideoMode* GetCurrentVideoMode()
    {
        return _current_mode;
    }

    const VideoMode* GetVideoModes()
    {
        return _vlist;
    }

    /**
     * Set the video mode given as parameter
     */
    stat_t SetVideoMode(const VideoMode *mode)
    {
        ENTER;
        for (size_t i = 0; i < _num_modes; i++) {
            if (mode == &_vlist[i]) {
                L4_Word_t reg = mode->Number;
                DOUT("set mode %lx\n", reg);
                stat_t err = _session->Put(&reg, 1);
                if (err == ERR_NONE) {
                    DOUT("current mode %lx\n", mode->Number);
                    _current_mode = (VideoMode*)mode;
                }
                EXIT;
                return err;
            }
        }
        return ERR_NOT_FOUND;
    }

    /**
     * Try to find and set the videomode with given characteristics.
     */
    stat_t SetVideoMode(UShort width, UShort height, UByte bpp)
    {
        // Parse the list of modes until we find one that suits
        for (size_t i = 0; i < _num_modes; i++) {
            if (_vlist[i].Xres == width && _vlist[i].Yres == height &&
                _vlist[i].Bpp == bpp) {
                return SetVideoMode(&_vlist[i]);
            }
        }
        return ERR_NOT_FOUND;
    }

    const VideoMode *GetCurrentMode() { return _current_mode; }

    stat_t AllocateFrameBuffer(const VideoMode* mode);
    stat_t AllocateFrameBuffer();
    void ReleaseFrameBuffer();

    addr_t GetFrameBuffer() { return _fb_base; }

    void Print()
    {
        System.Print("Board supports VBE %u.%u.\nVideo memory: %luKB\n", 
                     GetMajorVersion(), GetMinorVersion(),
                     GetTotalMemory() >> 12);
        System.Print("Supported modes:\n");
        for (size_t i = 0; i < _num_modes; i++) {
            _vlist[i].Print();
        }
    }

    /**
     * Get the pixel value at position (x,y).
     * The pixel is returned in the current video mode's format.
     * 
     * This is very slow. Applications looking for performance should
     * use direct frame buffer access instead.
     */
    UInt GetPixel(UShort x, UShort y);
    
    /**
     * Put the given pixel at position (x,y).
     * The pixel must be given in the current video mode's format.
     * 
     * This is very slow. Applications looking for performance should
     * use direct frame buffer access instead.
     */
    void PutPixel(UShort x, UShort y, UInt pixel);
    
    /**
     * Get the pixel value at position (x,y).
     * The pixel is returned in RGB format.
     * 
     * This is very slow. Applications looking for performance should
     * use direct frame buffer access instead.
     */
    RGBTriplet GetPixelRGB(UShort x, UShort y)
    { return GetCurrentMode()->PixelToRGB(GetPixel(x, y)); }
    
    /**
     * Put the given pixel at position (x,y).
     * The pixel must be given in RGB format.
     * 
     * This is very slow. Applications looking for performance should
     * use direct frame buffer access instead.
     */
    void PutPixelRGB(UShort x, UShort y, const RGBTriplet& pixel)
    { PutPixel(x, y, GetCurrentMode()->RGBToPixel(pixel)); }

};


#endif // ARC_VESASCREEN_H_

