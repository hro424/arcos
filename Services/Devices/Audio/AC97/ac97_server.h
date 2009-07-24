///
/// @brief  AC97 audio device, server-side implementation
/// @since  July 2009
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
///

#ifndef ARC_SERVICES_AUDIO_AC97_AC97_SERVER_H
#define ARC_SERVICES_AUDIO_AC97_AC97_SERVER_H

#include <Debug.h>
#include <System.h>
#include <PageAllocator.h>
#include <MemoryManager.h>
#include <Interrupt.h>
#include <List.h>
#include <AC97.h>
#include <arc/IO.h>
#include "regs.h"


class AC97ServerChannel
{
protected:
    /**
     * The base address of the memory-mapped registers of the bus master.
     */
    static addr_t   _bm_base;

    addr_t          _ch_base;

    addr_t ChannelBase() { return _ch_base; }

    /**
     * Reads the register of the bus master in 32-bit.
     */
    static UInt BMRead32(addr_t reg)
    { return *(UInt*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 32-bit.
     */
    static void BMWrite32(addr_t reg, UInt val)
    { *(UInt*)(_bm_base + reg) = val; }

    /**
     * Reads the register of the bus master in 16-bit.
     */
    static UShort BMRead16(addr_t reg) { return *(UShort*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 16-bit.
     */
    static void BMWrite16(addr_t reg, UShort val)
    { *(UShort*)(_bm_base + reg) = val; }

    /**
     * Reads the register of the bus master in 8-bit.
     */
    static UByte BMRead8(addr_t reg) { return *(UByte*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 8-bit.
     */
    static void BMWrite8(addr_t reg, UByte val)
    { *(UByte*)(_bm_base + reg) = val; }


public:
    static const size_t NUM_CHANNELS = 6;

    static void Initialize(addr_t bm) { _bm_base = bm; }

    /**
     * Places the value to the global control register.
     */
    static void SetGlobalControl(UInt val) { BMWrite32(AC97_IO_GLOB_CNT, val); }

    /**
     * Obtains the value of the global control register.
     */
    static UInt GetGlobalControl() { return BMRead32(AC97_IO_GLOB_CNT); }

    /**
     * Places the value to the global status register.
     */
    static void SetGlobalStatus(UInt val) { BMWrite32(AC97_IO_GLOB_STA, val); }

    /**
     * Obtains the value of the global status register.
     */
    static UInt GetGlobalStatus() { return BMRead32(AC97_IO_GLOB_STA); }

    AC97ServerChannel(UByte ch_base) : _ch_base(ch_base) {}
    virtual ~AC97ServerChannel() {}

    void SetBufDescBase(addr_t base)
    { BMWrite32(AC97_IO_BDBAR(ChannelBase()), base); }

    addr_t GetBufDescBase() { return BMRead32(AC97_IO_BDBAR(ChannelBase())); }

    UByte GetCurrentIndex() { return BMRead8(AC97_IO_CIV(ChannelBase())); }

    void SetLastValidIndex(UByte i) { BMWrite8(AC97_IO_LVI(ChannelBase()), i); }

    UByte GetLastValidIndex() { return BMRead8(AC97_IO_LVI(ChannelBase())); }

    void SetStatus(UShort stat) { BMWrite16(AC97_IO_SR(ChannelBase()), stat); }

    UShort GetStatus() { return BMRead16(AC97_IO_SR(ChannelBase())); }

    UShort GetPosition() { return BMRead16(AC97_IO_PICB(ChannelBase())); }

    UByte GetPrefetchedIndex() { return BMRead8(AC97_IO_PIV(ChannelBase())); }

    void SetControl(UByte val) { BMWrite8(AC97_IO_CR(ChannelBase()), val); }

    UByte GetControl() { return BMRead8(AC97_IO_CR(ChannelBase())); }

    void Activate() { SetControl(0x19); }

    void Deactivate() { SetControl(0); }

    void Print()
    { System.Print("base:%.8lX ch_base:%.8lX\n", _bm_base, ChannelBase()); }
};


class AC97ServerMixer
{
protected:
    /**
     * The base address of the memory-mapped registers of the mixer.
     */
    addr_t      _mixer_base;

    /**
     * Reads the register of the mixer.
     */
    UShort MixerRead(addr_t reg) { return *(UShort*)(_mixer_base + reg); }

    /**
     * Writes the value to the register of the mixer.
     */
    void MixerWrite(addr_t reg, UShort val)
    { *(UShort*)(_mixer_base + reg) = val; }

    AC97ServerMixer() {}

public:
    void Initialize(addr_t base) { _mixer_base = base; }

    void SetMasterVolume(UShort vol) { MixerWrite(AC97_IO_MASTER_VOL, vol); }

    UShort GetMasterVolume() { return MixerRead(AC97_IO_MASTER_VOL); }

    void SetMonoVolume(UShort vol) { MixerWrite(AC97_IO_MONO_VOL, vol); }

    UShort GetMonoVolume() { return MixerRead(AC97_IO_MONO_VOL); }

    void SetMasterTone(UShort tone) { MixerWrite(AC97_IO_MASTER_TONE, tone); }

    UShort GetMasterTone() { return MixerRead(AC97_IO_MASTER_TONE); }

    void SetPCBeepVolume(UShort vol) { MixerWrite(AC97_IO_PCBEEP_VOL, vol); }

    UShort GetPCBeepVolume() { return MixerRead(AC97_IO_PCBEEP_VOL); }

    void SetPCMOutVolume(UShort vol) { MixerWrite(AC97_IO_PCMOUT_VOL, vol); }

    UShort GetPCMOutVolume() { return MixerRead(AC97_IO_PCMOUT_VOL); }

    friend class AC97Device;
};


class AC97Device : public InterruptHandler
{
private:
    ///
    /// The base address of the mapped registers of AC97 audio
    ///
    addr_t              _mapped_io;

    ///
    /// The physical address of the mixer
    ///
    static const addr_t MAPPED_IO_BASE = 0xF0010000;

    ///
    /// The mixer object
    ///
    AC97ServerMixer             _mixer;

    ///
    /// The list of channels objects
    ///
    AC97ServerChannel*          _channels[AC97Channel::NUM_CHANNELS];

    ///
    /// The listeners of interrupt
    ///
    List<L4_ThreadId_t>         _listeners;

    ///
    /// Registers the base address of the mixer.  The old value is over-written.
    ///
    void SetMixerBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MMBAR, base & 0xFFFFFE00); }

    ///
    /// Registers the base address of the bus master.  The old value is over-
    /// written.
    ///
    void SetBusMasterBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MBBAR, base & 0xFFFFFF00); }

public:
    /// 
    /// Interrupt request number of the audio device.
    ///
    static const UInt   IRQ_AC97Server = 0xC;

    ///
    /// Initializes the instance.
    ///
    stat_t Initialize();

    ///
    /// Cleans up the instance.
    ///
    void Finalize()
    {
        ENTER;
        L4_Msg_t                    msg;
        Iterator<L4_ThreadId_t>&    it = _listeners.GetIterator();

        L4_Put(&msg, MSG_EVENT_TERMINATE, 0, 0, 0, 0);
        while (it.HasNext()) {
            Ipc::Send(it.Next(), &msg);
        }

        for (size_t i = 0; i < AC97Channel::NUM_CHANNELS; i++) {
            if (_channels[i] != 0) {
                delete _channels[i];
            }
        }
        pfree(_mapped_io, 1);
        EXIT;
    }

    ///
    /// Enables the interrupt.
    ///
    stat_t EnableInterrupt()
    {
        InterruptManager* imng = InterruptManager::Instance();
        return imng->Register(this, IRQ_AC97Server);
    }

    ///
    /// Disables the interrupt.
    ///
    void DisableInterrupt()
    {
        InterruptManager* imng = InterruptManager::Instance();
        imng->Deregister(IRQ_AC97Server);
    }

    ///
    /// Handles the interrupt request.
    ///
    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg)
    {
        L4_Msg_t                    event;
        Iterator<L4_ThreadId_t>&    it = _listeners.GetIterator();

        L4_Put(&event, MSG_EVENT_NOTIFY, 0, 0, 0, 0);
        while (it.HasNext()) {
            L4_ThreadId_t th = it.Next();
            Ipc::Send(th, &event);
        }
    }

    ///
    /// Obtains the channel of the type.
    ///
    AC97ServerChannel* Channel(UInt type)
    {
        if (0 <= type && type < AC97Channel::NUM_CHANNELS) {
            return _channels[type];
        }
        else {
            return 0;
        }
    }

    AC97ServerMixer* Mixer() { return &_mixer; }

    ///
    /// Adds the listener of the interrupt.
    ///
    void AddListener(L4_ThreadId_t tid) { _listeners.Add(tid); }

    ///
    /// Removes the listener from the listener list.
    ///
    void DelListener(L4_ThreadId_t tid) { _listeners.Remove(tid); }
};


#endif // ARC_SERVICES_AUDIO_AC97_AC97_SERVER_H

