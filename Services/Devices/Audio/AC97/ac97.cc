
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <arch/io.h>
#include "ac97.h"

void
AC97::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    DOUT("int %.8lX\n", tid.raw);
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
    SetTransferBaseAddress(BUFFER_ADDRESS);

    PCI_Write16(ICH_AUDIO, PCI_PCICMD,
                PCI_PCICMD_BME | PCI_PCICMD_MSE | PCI_PCICMD_IOSE);

    return ERR_NONE;
}

