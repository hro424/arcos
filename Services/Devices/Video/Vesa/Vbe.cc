
#include <Debug.h>
#include <System.h>
#include <MemoryManager.h>
#include <PageAllocator.h>
#include "Vbe.h"
#include "VbeInfoBlock.h"
#include "ModeInfoBlock.h"
#include "VideoMode.h"
#include "RealModeEmu.h"

stat_t
Vbe::MapLFBPage(L4_Word_t physaddress, L4_Word_t spaceaddress)
{
    /*
    L4_KernelInterfacePage_t *kip;
    L4_ThreadId_t   s0id;
    L4_Msg_t        msg;
    L4_Word_t       reg[2];
    L4_Fpage_t      page;
    L4_Fpage_t      acceptedpage;

    ENTER;

    kip = (L4_KernelInterfacePage_t *)L4_GetKernelInterface();
    s0id = L4_GlobalId(L4_ThreadIdUserBase(kip), 1);
    page = L4_FpageLog2(physaddress, PAGE_BITS);
    acceptedpage = L4_FpageLog2((L4_Word_t)spaceaddress, PAGE_BITS);

    reg[0] = page.raw;
    reg[1] = 0;
    L4_Put(&msg, MSG_SIGMA0, 2, reg, 0, 0);
    
    L4_Accept(L4_MapGrantItems(acceptedpage));
    stat_t err = Ipc::Call(s0id, &msg, &msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    
    EXIT;
    return err;
    */
    ENTER;

    DOUT("phys: %.8lX\n", physaddress);
    BREAK("");
    EXIT;
    return ERR_NONE;
}

void
Vbe::AllocateLFB(VideoMode* mode)
{
    _lfb_pages = (mode->xres() * mode->yres() * mode->bpp() *
                  mode->numberOfPages()) / PAGE_SIZE;

    _lfb_base = palloc_shm(_lfb_pages, L4_Myself(), L4_ReadWriteOnly);
    //_lfb_base = palloc(_lfb_pages);
    DOUT("Allocate LFB @ %.8lX %u pages\n", _lfb_base, _lfb_pages);
}

void
Vbe::ReleaseLFB()
{
    if (_lfb_base != 0) {
        pfree(_lfb_base, _lfb_pages);
    }
}

stat_t
Vbe::SetVideoMode(UInt n) {
    ENTER;

    if (n >= MAX_SUPPORTED_MODES) {
        return ERR_INVALID_ARGUMENTS;
    }

    VideoMode* mode = _video_mode[n];

    // Set the video mode using BIOS functions
    X86EMU_SETREG(AX, 0x4f02);
    X86EMU_SETREG(BX, mode->number() | VIDEO_MODE_USE_LFB);
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        return ERR_UNKNOWN;
    }
    // Now map the frame buffer to our address space
    // We have to map the pages one by one - so let's first calculate how many
    // pages to map.
    L4_Word_t lfb_phys = mode->lfbPhysicalAddress();
    DOUT("Map LFB page @ %.8lX\n", lfb_phys);

    ReleaseLFB();

    AllocateLFB(mode);

    for (UInt i = 0; i < _lfb_pages; i++) {
        /*
        stat_t err = MapLFBPage(lfb_phys + (PAGE_SIZE * i),
                                _lfb_base + (PAGE_SIZE * i));
        if (err != ERR_NONE) {
            pfree(_lfb_base);
            return err;
        }
        */
        stat_t err = Pager.Map(_lfb_base + (PAGE_SIZE * i), L4_ReadWriteOnly,
                               lfb_phys + (PAGE_SIZE * i), L4_nilthread);
        if (err != ERR_NONE) {
            ReleaseLFB();
            return err;
        }
    }

    System.Print("Set mode %dx%dx%dbpp\n", mode->xres(), mode->yres(),
                 mode->bpp());
    EXIT;
    return ERR_NONE;
}

void
Vbe::Initialize()
{
    ModeInfoBlock   mode_info;

    ReadVbeInfoBlock(&_vbe_info);

    const UShort* mode_list = _vbe_info.GetModeList();
    UInt i = 0;
    for (UShort* cur = (UShort*)mode_list;
         *cur != 0xFFFF && i < MAX_SUPPORTED_MODES;
         cur++) {
        if (ReadModeInfoBlock(&mode_info, *cur) != ERR_NONE) {
            continue;
        }

        if (mode_info.ModeAttributes & VIDEO_MODE_SUPPORTED &&
            mode_info.ModeAttributes & VIDEO_MODE_GRAPHIC &&
            mode_info.ModeAttributes & VIDEO_MODE_SUPPORTS_LFB) {
            _video_mode[i] = new VideoMode(*cur, &mode_info);
            i++;
        }
    }
    _num_modes = i;

    _lfb_base = 0;
    _lfb_pages = 0;
}

void
Vbe::Finalize()
{
    for (UInt i = 0; i < _num_modes; i++) {
        delete _video_mode[i];
    }
}

void
Vbe::Print()
{
    _vbe_info.Print();
    for (UInt i = 0; i < _num_modes; i++) {
        _video_mode[i]->Print();
    }
}

