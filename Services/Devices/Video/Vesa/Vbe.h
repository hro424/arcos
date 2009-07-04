#ifndef ARC_DEVICE_VIDEO_VBE_H
#define ARC_DEVICE_VIDEO_VBE_H

#include <Types.h>
#include <vesa/VideoMode.h>
#include "VbeInfoBlock.h"
#include "ModeInfoBlock.h"

class Vbe
{
private:
    static const UInt   MAX_SUPPORTED_MODES = 64;

    VbeInfoBlock        _vbe_info;

    VideoMode*          _video_mode[MAX_SUPPORTED_MODES];

    UInt                _num_modes;

    stat_t ReadVbeInfoBlock(VbeInfoBlock* block);
    ModeInfoBlock* GetModeInfoBlock(UInt number);

public:

    Vbe() {}

    ~Vbe() {}

    /**
     * Initializes the VBE interface.
     */
    stat_t Initialize();

    VideoMode* CreateVideoMode(UShort number, const ModeInfoBlock* block);

    /**
     * Cleans up the VBE interface.
     */
    void Finalize()
    {
        for (UInt i = 0; i < _num_modes; i++) {
            delete _video_mode[i];
        }

        // TODO: Enable that when x86emu unmaps the bios pages on cleanup.
        // pfree(main_mem);
        CleanupRealModeEmulator();
    }

    /**
     * Configures the video mode.
     */
    stat_t SetVideoMode(UInt n)
    {
        ENTER;

        if (n >= MAX_SUPPORTED_MODES) {
            return ERR_INVALID_ARGUMENTS;
        }

        VideoMode* mode = _video_mode[n];

        // Set the video mode using BIOS functions
        X86EMU_SETREG(AX, 0x4f02);
        X86EMU_SETREG(BX, mode->Number | VIDEO_MODE_USE_LFB);
        invokeInt10();
        if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
            return ERR_UNKNOWN;
        }

        System.Print("Set mode %dx%dx%dbpp\n", mode->Xres, mode->Yres,
                     mode->Bpp);
        EXIT;
        return ERR_NONE;
    }

    /**
     * Obtains the current video mode.
     */
    const VideoMode* GetVideoMode(UInt n) { return _video_mode[n]; }

    /**
     * Obtains the number of supported video modes.
     */
    UInt GetNumberOfVideoModes() { return _num_modes; }

    const VbeInfoBlock* Info() { return &_vbe_info; }

    /**
     * Displays the information of the VBE interface.
     */
    void Print()
    {
        _vbe_info.Print();
        for (UInt i = 0; i < _num_modes; i++) {
            _video_mode[i]->Print();
        }
    }
};

#endif // ARC_DEVICE_VIDEO_VBE_H

