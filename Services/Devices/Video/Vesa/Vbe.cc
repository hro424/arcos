
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include "Vbe.h"
#include "VbeInfoBlock.h"
#include "ModeInfoBlock.h"
#include <vesa/VideoMode.h>
#include "RealModeEmu.h"

stat_t
Vbe::ReadVbeInfoBlock(VbeInfoBlock* block)
{
    addr_t  block_addr;

    // First get the address of the VBE info block.
    X86EMU_SETREG(AX, 0x4f00);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, 0);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print("vesa: Cannot query VBE capabalities!\n");
        return ERR_NOT_FOUND;
    }
 
    memset(block, 0, sizeof(VbeInfoBlock));
    block_addr = RME_BASE + X86EMU_DATA_AREA_OFFSET;
    memcpy(block, (const void*)block_addr, sizeof(VbeInfoBlock));
    return ERR_NONE;
}

ModeInfoBlock*
Vbe::GetModeInfoBlock(UInt number)
{
    X86EMU_SETREG(AX, 0x4f01);
    X86EMU_SETREG(CX, number);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, sizeof(VbeInfoBlock));
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print("vesa: Cannot query mode %x!\n", number);
        return 0;
    }

    return (ModeInfoBlock*)(RME_BASE + X86EMU_DATA_AREA_OFFSET +
                           sizeof(VbeInfoBlock));
}

stat_t
Vbe::Initialize()
{
    ENTER;
    if (InitializeRealModeEmulator() != ERR_NONE) {
        return ERR_FATAL;
    }

    DOUT("RME initialized.\n");

    ReadVbeInfoBlock(&_vbe_info);

    const UShort* mode_list = _vbe_info.GetModeList();
    UInt i = 0;
    for (UShort* modep = (UShort*)mode_list;
         *modep != 0xFFFF && i < MAX_SUPPORTED_MODES;
         modep++) {
        ModeInfoBlock* buf = GetModeInfoBlock(*modep);
        if (buf == 0) {
            continue;
        }

        if (buf->ModeAttributes & VIDEO_MODE_SUPPORTED &&
            buf->ModeAttributes & VIDEO_MODE_GRAPHIC &&
            buf->ModeAttributes & VIDEO_MODE_SUPPORTS_LFB) {
            _video_mode[i] = CreateVideoMode(*modep, buf);
            i++;
        }
    }
    _num_modes = i;

    EXIT;
    return ERR_NONE;
}

VideoMode*
Vbe::CreateVideoMode(UShort number, const ModeInfoBlock *info)
{
    VideoMode* vmode = new VideoMode();

    vmode->Number = number;
    vmode->Xres = info->XResolution;
    vmode->Yres = info->YResolution;
    vmode->Bpp = info->BitsPerPixel;
    vmode->RedMask = 0;
    vmode->GreenMask = 0;
    vmode->BlueMask = 0;
    vmode->RedMaskSize = info->RedMaskSize;
    vmode->GreenMaskSize = info->GreenMaskSize;
    vmode->BlueMaskSize = info->BlueMaskSize;
    vmode->RedFieldPos = info->RedFieldPosition;
    vmode->GreenFieldPos = info->GreenFieldPosition;
    vmode->BlueFieldPos = info->BlueFieldPosition;
    vmode->NumberOfPages = info->NumberOfImagePages + 1;
    vmode->LFBAddress = info->PhysBasePtr;

    for (int i = 0; i < vmode->RedMaskSize; i++) {
        vmode->RedMask |= (1 << vmode->RedFieldPos + i);
    }

    for (int i = 0; i < vmode->GreenMaskSize; i++) {
        vmode->GreenMask |= (1 << vmode->GreenFieldPos + i);
    }

    for (int i = 0; i < vmode->BlueMaskSize; i++) {
        vmode->BlueMask |= (1 << vmode->BlueFieldPos + i);
    }

    vmode->RedLoss = 0xff >> (8 - vmode->RedMaskSize) << (8 - vmode->RedMaskSize);
    vmode->GreenLoss = 0xff >> (8 - vmode->GreenMaskSize) << (8 - vmode->GreenMaskSize);
    vmode->BlueLoss = 0xff >> (8 - vmode->BlueMaskSize) << (8 - vmode->BlueMaskSize);

    return vmode;
}


