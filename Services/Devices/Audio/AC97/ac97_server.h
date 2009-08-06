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
#include <Server.h>
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

    void SetStatus32(UInt stat) { BMWrite32(AC97_IO_CIV(ChannelBase()), stat); }

    UInt GetStatus32() { return BMRead32(AC97_IO_CIV(ChannelBase())); }

    UShort GetPosition() { return BMRead16(AC97_IO_PICB(ChannelBase())); }

    UByte GetPrefetchedIndex() { return BMRead8(AC97_IO_PIV(ChannelBase())); }

    void SetControl(UByte val) { BMWrite8(AC97_IO_CR(ChannelBase()), val); }

    UByte GetControl() { return BMRead8(AC97_IO_CR(ChannelBase())); }

    void Reset() { SetControl(2); }

    void Activate() { ENTER; SetControl(0x19); EXIT; }

    void Deactivate() { ENTER; SetControl(0); EXIT; }

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

    void SetReset(UShort val) { MixerWrite(AC97_IO_RESET, val); }

    UShort GetReset() { return MixerRead(AC97_IO_RESET); }

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

    UShort GetExtAudioID() { return MixerRead(AC97_IO_EXT_AUDIO); }

    UShort GetExtAudioStatus() { return MixerRead(AC97_IO_EXT_AUDIO_CTRL); }

    void SetExtAudioStatus(UShort stat)
    { MixerWrite(AC97_IO_EXT_AUDIO_CTRL, stat); }

    void SetPCMFrontSamplingRate(UShort rate)
    { MixerWrite(AC97_IO_PCM_FRONT_DAC, rate); }

    UShort GetPCMFrontSamplingRate()
    { return MixerRead(AC97_IO_PCM_FRONT_DAC); }

    void SetPCMSurrSamplingRate(UShort rate)
    { MixerWrite(AC97_IO_PCM_SURR_DAC, rate); }

    UShort GetPCMSurrSamplingRate()
    { return MixerRead(AC97_IO_PCM_SURR_DAC); }

    void SetPCMLFESamplingRate(UShort rate)
    { MixerWrite(AC97_IO_PCM_LFE_DAC, rate); }

    UShort GetPCMLFESamplingRate() { return MixerRead(AC97_IO_PCM_LFE_DAC); }

    void SetPCMLRSamplingRate(UShort rate)
    { MixerWrite(AC97_IO_PCM_LR_ADC, rate); }

    UShort GetPCMLRSamplingRate() { return MixerRead(AC97_IO_PCM_LR_ADC); }

    friend class AC97Device;
};


class AC97Device : public InterruptHandler
{
private:
    ///
    /// The base address of the mapped registers of AC97 audio
    ///
    addr_t                      _mapped_io;

    ///
    /// The physical address of the mixer
    ///
    static const addr_t         MAPPED_IO_BASE = 0xF0010000;

    ///
    /// The mixer object
    ///
    AC97ServerMixer             _mixer;

    ///
    /// The list of channels objects
    ///
    static AC97ServerChannel    _channels[AC97Channel::NUM_CHANNELS];

    ///
    /// The listeners of interrupt
    ///
    //L4_ThreadId_t IS_PERSISTENT _listener;

    //Bool IS_PERSISTENT          _int_enabled;

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

    void Recover();

    ///
    /// Cleans up the instance.
    ///
    void Finalize();

    ///
    /// Enables the interrupt.
    ///
    stat_t EnableInterrupt();

    ///
    /// Disables the interrupt.
    ///
    void DisableInterrupt();

    ///
    /// Handles the interrupt request.
    ///
    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg);

    ///
    /// Obtains the channel of the type.
    ///
    AC97ServerChannel* Channel(UInt type)
    {
        if (0 <= type && type < AC97Channel::NUM_CHANNELS) {
            return &_channels[type];
        }
        else {
            return 0;
        }
    }

    AC97ServerMixer* Mixer() { return &_mixer; }

    ///
    /// Adds the listener of the interrupt.
    ///
    //void AddListener(L4_ThreadId_t& tid) { _listener = tid; }
    void AddListener(L4_ThreadId_t& tid);

    ///
    /// Removes the listener from the listener list.
    ///
    //void DelListener() { _listener = L4_nilthread; }
    void DelListener();
};

#endif // ARC_SERVICES_AUDIO_AC97_AC97_SERVER_H

