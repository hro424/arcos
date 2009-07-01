#include <Debug.h>
#include <System.h>
#include <String.h>

#include "VbeInfoBlock.h"
#include "ModeInfoBlock.h"
#include "RealModeEmu.h"

stat_t
ReadModeInfoBlock(ModeInfoBlock* block, UInt number)
{
    addr_t block_addr;

    X86EMU_SETREG(AX, 0x4f01);
    X86EMU_SETREG(CX, number);
    X86EMU_SETREG(ES, X86EMU_DATA_AREA_OFFSET >> 4);
    X86EMU_SETREG(DI, sizeof(VbeInfoBlock));
    invokeInt10();
    if (!VESA_VBE_SUCCESS(X86EMU_GETREG(AX))) {
        System.Print("vesa: Cannot query mode %x!\n", number);
        return ERR_NOT_FOUND;
    }

    memset(block, 0, sizeof(ModeInfoBlock));
    block_addr = _rme_base + X86EMU_DATA_AREA_OFFSET + sizeof(VbeInfoBlock);
    memcpy(block, (const void*)block_addr, sizeof(ModeInfoBlock));
    return ERR_NONE;
}

