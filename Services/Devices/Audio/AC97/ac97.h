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
    addr_t      _mixer_base;
    addr_t      _bm_base;
    static const addr_t MIXER_ADDRESS = 0xF0010000;
    static const addr_t BUFFER_ADDRESS = 0xF0020000;

    void SetMixerBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MMBAR, base & 0xFFFFFE00); }

    void SetTransferBaseAddress(addr_t base)
    { PCI_Write32(ICH_AUDIO, PCI_MBBAR, base & 0xFFFFFF00); }

    UShort MixerRead(addr_t reg) { return *(UShort*)(_mixer_base + reg); }

    void MixerWrite(addr_t reg, UShort val)
    { *(UShort*)(_mixer_base + reg) = val; }

    UInt BMRead32(addr_t reg) { return *(UInt*)(_bm_base + reg); }

    void BMWrite32(addr_t reg, UInt val) { *(UInt*)(_bm_base + reg) = val; }

    UShort BMRead16(addr_t reg) { return *(UShort*)(_bm_base + reg); }

    void BMWrite16(addr_t reg, UShort val) { *(UShort*)(_bm_base + reg) = val; }

    UByte BMRead8(addr_t reg) { return *(UByte*)(_bm_base + reg); }

    void BMWrite8(addr_t reg, UByte val) { *(UByte*)(_bm_base + reg) = val; }

public:
    static const UInt   IRQ_AC97 = 0xC;

    stat_t Initialize();

    void Finalize()
    {
        pfree(_bm_base, 1);
        pfree(_mixer_base, 1);
    }

    void SetGlobalControl(UInt val) { BMWrite32(0x2C, val); }

    UInt GetGlobalControl() { return BMRead32(0x2C); }

    void SetGlobalStatus(UInt val)
    { BMWrite32(0x30, val); }

    UInt GetGlobalStatus() { return BMRead32(0x30); }

    void SetPCMOutBuffer(addr_t base) { BMWrite32(0x10, base); }

    addr_t GetPCMOutBuffer() { return BMRead32(0x10); }

    UByte GetPCMOutCurrentIndex() { return BMRead8(0x14); }

    void SetPCMOutLastValidIndex(UByte i) { BMWrite8(0x15, i); }

    UByte GetPCMOutLastValidIndex() { return BMRead8(0x15); }

    void SetPCMOutStatus(UShort stat) { BMWrite16(0x16, stat); }

    UShort GetPCMOutStatus() { return BMRead16(0x16); }

    UShort GetPCMOutPosition() { return BMRead16(0x18); }

    UByte GetPCMOutPrefetchedIndex() { return BMRead8(0x1A); }

    void SetPCMOutControl(UByte val) { BMWrite8(0x1B, val); }

    UByte GetPCMOutControl() { return BMRead8(0x1B); }

    void ResetPCMOut() { SetPCMOutControl(0x2); }



    void SetMasterVolume(UShort vol) { MixerWrite(AMREG_MASTER_VOL, vol); }

    UShort GetMasterVolume() { return MixerRead(AMREG_MASTER_VOL); }

    void SetMonoVolume(UShort vol) { MixerWrite(AMREG_MONO_VOL, vol); }

    UShort GetMonoVolume() { return MixerRead(AMREG_MONO_VOL); }

    void SetMasterTone(UShort tone) { MixerWrite(AMREG_MASTER_TONE, tone); }

    UShort GetMasterTone() { return MixerRead(AMREG_MASTER_TONE); }

    void SetPCBeepVolume(UShort vol) { MixerWrite(AMREG_PCBEEP_VOL, vol); }

    UShort GetPCBeepVolume() { return MixerRead(AMREG_PCBEEP_VOL); }

    void SetPCMOutVolume(UShort vol) { MixerWrite(AMREG_PCMOUT_VOL, vol); }

    UShort GetPCMOutVolume() { return MixerRead(AMREG_PCMOUT_VOL); }


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

    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg);
};

#endif // ARC_SERVICES_AUDIO_AC97_AC97_H

