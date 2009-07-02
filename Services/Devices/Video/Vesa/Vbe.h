#ifndef ARC_DEVICE_VIDEO_VBE_H
#define ARC_DEVICE_VIDEO_VBE_H

#include <Types.h>
#include "VbeInfoBlock.h"
#include "VideoMode.h"

class Vbe
{
private:
    static const UInt   MAX_SUPPORTED_MODES = 64;

    VbeInfoBlock        _vbe_info;

    VideoMode*          _video_mode[MAX_SUPPORTED_MODES];

    UInt                _num_modes;

    /**
     * Linear frame buffer base address
     */
    addr_t              _lfb_base;

    /**
     * Number of pages for LFB
     */
    size_t              _lfb_pages;

    stat_t MapLFBPage(L4_Word_t physaddress, L4_Word_t spaceaddress);

    void AllocateLFB(VideoMode* mode);
    void ReleaseLFB();

public:
    /**
     * Initializes the VBE interface.
     */
    void Initialize();

    /**
     * Cleans up the VBE interface.
     */
    void Finalize();

    /**
     * Configures the video mode.
     */
    stat_t SetVideoMode(UInt n);

    /**
     * Obtains the current video mode.
     */
    const VideoMode* GetVideoMode(UInt n) { return _video_mode[n]; }

    addr_t GetFrameBuffer() { return _lfb_base; }

    /**
     * Displays the information of the VBE interface.
     */
    void Print();
};

#endif // ARC_DEVICE_VIDEO_VBE_H

