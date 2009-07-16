#ifndef ARC_SERVICES_AUDIO_AC97_AC97_H
#define ARC_SERVICES_AUDIO_AC97_AC97_H

#include <Debug.h>
#include <System.h>
#include <PageAllocator.h>
#include <MemoryManager.h>
#include <Interrupt.h>
#include <arc/IO.h>
#include "regs.h"

class AC97 : public InterruptHandler
{
public:
    /**
     * The base address of the memory-mapped registers of the mixer.
     */
    addr_t      _mixer_base;

    /**
     * The base address of the memory-mapped registers of the bus master.
     */
    addr_t      _bm_base;

    /**
     * The physical address of the mixer.
     */
    static const addr_t MIXER_ADDRESS = 0xF0010000;

    /**
     * The physical address of the bus master.
     */
    static const addr_t BUFFER_ADDRESS = 0xF0020000;

    /**
     * Registers the base address of the mixer.  The old value is over-written.
     */
    void SetMixerBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MMBAR, base & 0xFFFFFE00); }

    /**
     * Registers the base address of the bus master.  The old value is over-
     * written.
     */
    void SetBusMasterBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MBBAR, base & 0xFFFFFF00); }

    /**
     * Reads the register of the mixer.
     */
    UShort MixerRead(addr_t reg) { return *(UShort*)(_mixer_base + reg); }

    /**
     * Writes the value to the register of the mixer.
     */
    void MixerWrite(addr_t reg, UShort val)
    { *(UShort*)(_mixer_base + reg) = val; }

    /**
     * Reads the register of the bus master in 32-bit.
     */
    UInt BMRead32(addr_t reg) { return *(UInt*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 32-bit.
     */
    void BMWrite32(addr_t reg, UInt val) { *(UInt*)(_bm_base + reg) = val; }

    /**
     * Reads the register of the bus master in 16-bit.
     */
    UShort BMRead16(addr_t reg) { return *(UShort*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 16-bit.
     */
    void BMWrite16(addr_t reg, UShort val) { *(UShort*)(_bm_base + reg) = val; }

    /**
     * Reads the register of the bus master in 8-bit.
     */
    UByte BMRead8(addr_t reg) { return *(UByte*)(_bm_base + reg); }

    /**
     * Writes the value to the register of the bus master in 8-bit.
     */
    void BMWrite8(addr_t reg, UByte val) { *(UByte*)(_bm_base + reg) = val; }

public:
    /**
     * Interrupt request number of the audio device.
     */
    static const UInt   IRQ_AC97 = 0xC;

    /**
     * Initializes the instance.
     */
    stat_t Initialize();

    /**
     * Cleans up the instance.
     */
    void Finalize()
    {
        pfree(_bm_base, 1);
        pfree(_mixer_base, 1);
    }

    /**
     * Places the value to the global control register.
     */
    void SetGlobalControl(UInt val) { BMWrite32(AC97_IO_GLOB_CNT, val); }

    /**
     * Obtains the value of the global control register.
     */
    UInt GetGlobalControl() { return BMRead32(AC97_IO_GLOB_CNT); }

    /**
     * Places the value to the global status register.
     */
    void SetGlobalStatus(UInt val)
    { BMWrite32(AC97_IO_GLOB_STA, val); }

    /**
     * Obtains the value of the global status register.
     */
    UInt GetGlobalStatus() { return BMRead32(AC97_IO_GLOB_STA); }


    void SetPCMOutBufDescBase(addr_t base) { BMWrite32(AC97_IO_POBDBAR, base); }

    addr_t GetPCMOutBufDescBase() { return BMRead32(AC97_IO_POBDBAR); }

    UByte GetPCMOutCurrentIndex() { return BMRead8(AC97_IO_POCIV); }

    void SetPCMOutLastValidIndex(UByte i) { BMWrite8(AC97_IO_POLVI, i); }

    UByte GetPCMOutLastValidIndex() { return BMRead8(AC97_IO_POLVI); }

    void SetPCMOutStatus(UShort stat) { BMWrite16(AC97_IO_POSR, stat); }

    UShort GetPCMOutStatus() { return BMRead16(AC97_IO_POSR); }

    UShort GetPCMOutPosition() { return BMRead16(AC97_IO_POPICB); }

    UByte GetPCMOutPrefetchedIndex() { return BMRead8(AC97_IO_POPIV); }

    void SetPCMOutControl(UByte val) { BMWrite8(AC97_IO_POCR, val); }

    UByte GetPCMOutControl() { return BMRead8(AC97_IO_POCR); }

    void ResetPCMOut() { SetPCMOutControl(AC97_IO_CR_RR); }



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


    stat_t EnableInterrupt()
    {
        InterruptManager* imng = InterruptManager::Instance();
        return imng->Register(this, IRQ_AC97);
    }

    void DisableInterrupt()
    {
        InterruptManager* imng = InterruptManager::Instance();
        imng->Deregister(IRQ_AC97);
    }

    /**
     * Handles the interrupt request.
     */
    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg);
};

#endif // ARC_SERVICES_AUDIO_AC97_AC97_H

