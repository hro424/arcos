
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <arch/io.h>
#include "ac97.h"

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

    // Register buffers to the descriptors
    addr_t ptr = Pager.Phys((addr_t)test_buf);
    for (UInt i = 0; i < DESC_LENGTH; i += 2) {
        test_desc[i] = ptr;
        test_desc[i + 1] = 0x100;
        ptr += 0x100;
    }
}


stat_t
AC97::Initialize()
{
    System.Print("VendorID:%.4lX DeviceID:%.4lX SVID:%.4lX SID:%.4lX BCC:%.2lX SCC:%.2lX\n",
                 PCI_Read16(ICH_AUDIO, PCI_VID),
                 PCI_Read16(ICH_AUDIO, PCI_DID),
                 PCI_Read16(ICH_AUDIO, PCI_SVID),
                 PCI_Read16(ICH_AUDIO, PCI_SID),
                 PCI_Read8(ICH_AUDIO, PCI_BCC),
                 PCI_Read8(ICH_AUDIO, PCI_SCC));

    DOUT("irq: %.2lX\n", PCI_Read8(ICH_AUDIO, PCI_INT_LN));

    // Disable the bus master to setup the bus master base address
    PCI_Write16(ICH_AUDIO, PCI_PCICMD, 0);
    PCI_Write8(ICH_AUDIO, PCI_CFG, 0);

    _mixer_base = palloc_shm(1, L4_Myself(), L4_ReadWriteOnly);
    Pager.Map(_mixer_base, L4_ReadWriteOnly, MIXER_ADDRESS, L4_nilthread);
    SetMixerBaseAddress(MIXER_ADDRESS);

    _bm_base = palloc_shm(1, L4_Myself(), L4_ReadWriteOnly);
    Pager.Map(_bm_base, L4_ReadWriteOnly, BUFFER_ADDRESS, L4_nilthread);
    SetBusMasterBaseAddress(BUFFER_ADDRESS);

    PCI_Write16(ICH_AUDIO, PCI_PCICMD,
                PCI_PCICMD_BME | PCI_PCICMD_MSE | PCI_PCICMD_IOSE);

    TestInit();

    return ERR_NONE;
}

