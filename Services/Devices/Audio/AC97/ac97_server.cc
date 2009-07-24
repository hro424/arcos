
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <arch/io.h>
#include "ac97_server.h"

addr_t AC97ServerChannel::_bm_base;

/*
void
AC97::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    DOUT("int %lX\n", tid.raw >> 14);
    DOUT("stat PCI:%.4lX AC97:%.8lX PCM:%.4lX\n",
         PCI_Read16(ICH_AUDIO, PCI_PCISTS), GetGlobalStatus(),
         GetPCMOutStatus());
    DOUT("last:0x%X current:0x%.2lX prefetch:0x%.2lX position:0x%.4lX\n",
         GetPCMOutLastValidIndex(), GetPCMOutCurrentIndex(),
         GetPCMOutPrefetchedIndex(), GetPCMOutPosition());

    SetPCMOutStatus(0x4);

    UByte index = GetPCMOutLastValidIndex();
    int start;
    int end;
    if (index == (DESC_LENGTH - 1)) {
        index = DESC_LENGTH / 2 - 1;
        start = 0;
        end = 2048;
    }
    else {
        index = DESC_LENGTH - 1;
        start = 2048;
        end = 4096;
    }

    bool ceil = false;
    int sample = 0;
    int sample_step = 4;
    for (int i = start; i < end; i++) {
        if (!ceil) {
            sample += sample_step;
            if (sample > 0xFF) {
                sample = 0xFF;
                ceil = 1;
            }
            else {
                sample -= sample_step;
                if (sample < 0) {
                    sample = 0; 
                    ceil = 0;
                }
            }
            test_buf[i] = sample;
        }
    }

    // Reset the index of buffer descriptors
    SetPCMOutLastValidIndex(index);

    DOUT("stat PCI:%.4lX AC97:%.8lX PCM:%.4lX\n",
         PCI_Read16(ICH_AUDIO, PCI_PCISTS), GetGlobalStatus(),
         GetPCMOutStatus());
    DOUT("last:0x%X current:%.2lX prefetch:%.2lX position:%.4lX\n",
         GetPCMOutLastValidIndex(), GetPCMOutCurrentIndex(),
         GetPCMOutPrefetchedIndex(), GetPCMOutPosition());
}

void
AC97::TestInit()
{
    SetPCMOutLastValidIndex(DESC_LENGTH / 2 - 1);

    test_desc = (UInt*)palloc(1);
    Pager.Map((addr_t)test_desc, L4_ReadWriteOnly);
    SetPCMOutBufDescBase(Pager.Phys((addr_t)test_desc));

    test_buf = (char*)palloc(1);
    Pager.Map((addr_t)test_buf, L4_ReadWriteOnly);

    // Register buffers to the descriptor table
    addr_t ptr = Pager.Phys((addr_t)test_buf);
    for (UInt i = 0; i < DESC_LENGTH; i += 2) {
        test_desc[i] = ptr;
        test_desc[i + 1] = 0x100;
        ptr += 0x100;
    }
}
*/


stat_t
AC97Device::Initialize()
{
    System.Print("Initialize AC97 Audio Device\n");
    System.Print("VendorID:%.4lX DeviceID:%.4lX SVID:%.4lX SID:%.4lX BCC:%.2lX SCC:%.2lX\n",
                 PCI_Read16(ICH_AUDIO, PCI_VID),
                 PCI_Read16(ICH_AUDIO, PCI_DID),
                 PCI_Read16(ICH_AUDIO, PCI_SVID),
                 PCI_Read16(ICH_AUDIO, PCI_SID),
                 PCI_Read8(ICH_AUDIO, PCI_BCC),
                 PCI_Read8(ICH_AUDIO, PCI_SCC));

    //_irq = PCI_Read8(ICH_AUDIO, PCI_INT_LN);

    // Disable the bus master to setup the bus master base address
    PCI_Write16(ICH_AUDIO, PCI_PCICMD, 0);
    PCI_Write8(ICH_AUDIO, PCI_CFG, 0);

    // The mapped registers for mixer require 512 bytes.
    // The mapped registers for the bus master require 256 bytes.
    _mapped_io = palloc_shm(1, L4_Myself(), L4_ReadWriteOnly);
    Pager.Map(_mapped_io, L4_ReadWriteOnly, MAPPED_IO_BASE, L4_nilthread);
    SetMixerBaseAddress(MAPPED_IO_BASE);
    SetBusMasterBaseAddress(MAPPED_IO_BASE + 1024);

    _mixer.Initialize(_mapped_io);
    AC97ServerChannel::Initialize(_mapped_io + 1024);

    _channels[0] = new AC97ServerChannel(AC97_IO_PIBASE);
    _channels[1] = new AC97ServerChannel(AC97_IO_POBASE);
    _channels[2] = new AC97ServerChannel(AC97_IO_MCBASE);
    _channels[3] = new AC97ServerChannel(AC97_IO_MC2BASE);
    _channels[4] = new AC97ServerChannel(AC97_IO_PI2BASE);
    _channels[5] = new AC97ServerChannel(AC97_IO_SPBASE);

    for (int i = 0; i < 6; i++) {
        _channels[i]->Print();
    }

    // Activate the bus master
    PCI_Write16(ICH_AUDIO, PCI_PCICMD,
                PCI_PCICMD_BME | PCI_PCICMD_MSE | PCI_PCICMD_IOSE);

    //TestInit();

    return ERR_NONE;
}

