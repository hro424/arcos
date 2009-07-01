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
#include <vesa/RGBTriplet.h>
#include <vesa/VideoMode.h>
#include <sys/Config.h>

///
/// Controls the physical screen, provides routines to draw on it or to get
/// the framebuffer for direct access.
/// 
/// This class is not instanciable. Instead, its singleton is made available
/// (i.e. mapped) by the vesa server on Screen::Init() invokation. It is
/// immediatly followed by the array of supported video modes.
///
class Screen
{
private:
    static Screen*  _self;

    /**
     * Cannot be instantiated
     */
    Screen();
    Screen(const Screen& s);
    virtual ~Screen();
    
protected:    
    static const char* const    SERVER_NAME;
    static L4_ThreadId_t        _server;

    ///
    /// VBE version
    ///
    UShort  _version;

    ///
    /// Size of video memory
    ///
    UInt    _total_memory;

    /*
     * Cache the number of bytes per pixel and x resolution for the current
     * mode - this makes offsets calculation faster.
     */
    UByte   _bpp;
    UShort  _xres;
    
    Int     _current_mode;
    UByte   _num_modes;

    ///
    /// Base address of the frame buffer (client side)
    ///
    UByte*  _base;

    /**
     * Gives the offset (in bytes) to reach pixel (x,y)
     * from the framebuffer.
     */
    UInt PixelOffset(UShort x, UShort y);

    UInt GetPixel8(UShort x, UShort y);
    
    UInt GetPixel16(UShort x, UShort y);

    UInt GetPixel24(UShort x, UShort y);

    UInt GetPixel32(UShort x, UShort y);

    void PutPixel8(UShort x, UShort y, UInt pixel);

    void PutPixel16(UShort x, UShort y, UInt pixel);

    void PutPixel24(UShort x, UShort y, UInt pixel);

    void PutPixel32(UShort x, UShort y, UInt pixel);
   
    /**
     * Set the video mode which index in the list of supported modes
     * is given as argument.
     */
    stat_t SetVideoMode(UInt index);
        
public:
    /**
     * This is a client function.
     *
     * Initialization function. Calling this function makes the client
     * application ready to use the VESA driver. It must be called before
     * doing anything.
     */
    static stat_t Initialize();
    
    /**
     * Returns the unique instance of screen.
     */
    static Screen* const GetInstance();
    
    /**
     * Vesa major version
     */
    UByte MajorVersion();

    /**
     * Vesa minor version
     */
    UByte MinorVersion();

    ///
    /// Obtains the total amount of video memory in bytes.
    ///
    UInt TotalMemory();

    void printInfo();
    
    size_t GetNbSupportedModes();

    const VideoMode *const GetSupportedModes() const;
    
    /**
     * Set the video mode given as parameter
     */
    stat_t SetVideoMode(const VideoMode *const mode);

    /**
     * Try to find and set the videomode with given characteristics.
     */
    stat_t SetVideoMode(UShort width, UShort height, UByte bpp);

    const VideoMode *GetCurrentMode();

    UByte *GetFrameBuffer();
    
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
    RGBTriplet GetPixelRGB(UShort x, UShort y);
    
    /**
     * Put the given pixel at position (x,y).
     * The pixel must be given in RGB format.
     * 
     * This is very slow. Applications looking for performance should
     * use direct frame buffer access instead.
     */
    void PutPixelRGB(UShort x, UShort y, const RGBTriplet& pixel);

    stat_t Connect();
    stat_t Disconnect();
    stat_t Open();
    stat_t Close();
};


inline UByte
Screen::MajorVersion()
{
    return _version >> 8;
}

inline UByte
Screen::MinorVersion()
{
    return _version & 0xff;
}

inline UInt
Screen::TotalMemory()
{
    return _total_memory << 16;
}

inline size_t
Screen::GetNbSupportedModes()
{
    return _num_modes;
}

inline const VideoMode* const
Screen::GetSupportedModes() const
{
    /**
     * The supported modes are stored right after the intance of screen.
     */
    return (const VideoMode* const)(this + 1);
}

inline const VideoMode*
Screen::GetCurrentMode()
{
    ENTER;
    return &GetSupportedModes()[_current_mode];
}

inline UByte*
Screen::GetFrameBuffer()
{
    return _base;
}

inline RGBTriplet
Screen::GetPixelRGB(UShort x, UShort y)
{
    return GetCurrentMode()->PixelToRGB(GetPixel(x, y));
}

inline void
Screen::PutPixelRGB(UShort x, UShort y, RGBTriplet pixel)
{
    ENTER;
    PutPixel(x, y, GetCurrentMode()->RGBToPixel(pixel));
}

#endif // ARC_VESASCREEN_H_

