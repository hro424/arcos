
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <arch/io.h>
#include "ac97_server.h"

addr_t AC97ServerChannel::_bm_base;

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

    // Activate the bus master
    PCI_Write16(ICH_AUDIO, PCI_PCICMD,
                PCI_PCICMD_BME | PCI_PCICMD_MSE | PCI_PCICMD_IOSE);

    for (int i = 0; i < 6; i++) {
        _channels[i]->Reset();
        _channels[i]->Print();
    }

    return ERR_NONE;
}

