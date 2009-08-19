
#include <Debug.h>
#include <System.h>
#include <String.h>
#include <Server.h>
#include <arch/io.h>
#include "ac97_server.h"

static L4_ThreadId_t IS_PERSISTENT  _listener;
static Bool IS_PERSISTENT           _int_enabled;

addr_t AC97ServerChannel::_bm_base;

AC97ServerChannel AC97Device::_channels[AC97Channel::NUM_CHANNELS] =
{
    AC97ServerChannel(AC97_IO_PIBASE),
    AC97ServerChannel(AC97_IO_POBASE),
    AC97ServerChannel(AC97_IO_MCBASE),
    AC97ServerChannel(AC97_IO_MC2BASE),
    AC97ServerChannel(AC97_IO_PI2BASE),
    AC97ServerChannel(AC97_IO_SPBASE)
};

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

    SetMixerBaseAddress(MAPPED_IO_BASE);
    SetBusMasterBaseAddress(MAPPED_IO_BASE + 1024);

    // Activate the bus master
    PCI_Write16(ICH_AUDIO, PCI_PCICMD,
                PCI_PCICMD_BME | PCI_PCICMD_MSE | PCI_PCICMD_IOSE);

    // The mapped registers for mixer require 512 bytes.
    // The mapped registers for the bus master require 256 bytes.
    _mapped_io = palloc_shm(1, L4_Myself(), L4_ReadWriteOnly);
    Pager.Map(_mapped_io, L4_ReadWriteOnly, MAPPED_IO_BASE, L4_nilthread);
    _mixer.Initialize(_mapped_io);
    AC97ServerChannel::Initialize(_mapped_io + 1024);
    DOUT("GLOB_CNT:%.8lX\n", AC97ServerChannel::GetGlobalControl());
    AC97ServerChannel::ColdReset();

    for (int i = 0; i < 6; i++) {
        _channels[i].Reset();
        _channels[i].Print();
    }

    _int_enabled = FALSE;

    return ERR_NONE;
}

void
AC97Device::Finalize()
{
    ENTER;
    L4_Msg_t                    msg;

    L4_Put(&msg, MSG_EVENT_TERMINATE, 0, 0, 0, 0);
    Ipc::Send(_listener, &msg);

    pfree(_mapped_io, 1);
    EXIT;
}


void
AC97Device::Recover()
{
    _mapped_io = palloc_shm(1, L4_Myself(), L4_ReadWriteOnly);
    Pager.Map(_mapped_io, L4_ReadWriteOnly, MAPPED_IO_BASE, L4_nilthread);
    _mixer.Initialize(_mapped_io);
    AC97ServerChannel::Initialize(_mapped_io + 1024);
    if (_int_enabled) {
        EnableInterrupt();
    }
}


void
AC97Device::AddListener(L4_ThreadId_t& tid)
{
    _listener = tid;
}


void
AC97Device::DelListener()
{
    _listener = L4_nilthread;
}


stat_t
AC97Device::EnableInterrupt()
{
    stat_t err;
    err = InterruptManager::Instance()->Register(this, IRQ_AC97Server);
    _int_enabled = TRUE;
    return err;
}


void
AC97Device::DisableInterrupt()
{
    InterruptManager::Instance()->Deregister(IRQ_AC97Server);
    _int_enabled = FALSE;
}


void
AC97Device::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg)
{
    L4_Msg_t    event;

    L4_Put(&event, MSG_EVENT_NOTIFY, 0, 0, 0, 0);
    L4_Append(&event, AC97ServerChannel::GetGlobalStatus());
    Ipc::Call(_listener, &event, &event);
}

