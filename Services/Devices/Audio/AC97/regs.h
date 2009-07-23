#ifndef ARC_SERVICES_AUDIO_AC97_REGS_H
#define ARC_SERVICES_AUDIO_AC97_REGS_H

#define AC97_IO_RESET                   0x00
#define AC97_IO_MASTER_VOL              0x02
#define AC97_IO_AUXOUT_VOL              0x04
#define AC97_IO_MONO_VOL                0x06
#define AC97_IO_MASTER_TONE             0x08
#define AC97_IO_PCBEEP_VOL              0x0A
#define AC97_IO_PHONE_VOL               0x0C
#define AC97_IO_MIC_VOL                 0x0E
#define AC97_IO_LINEIN_VOL              0x10
#define AC97_IO_CD_VOL                  0x12
#define AC97_IO_VIDEO_VOL               0x14
#define AC97_IO_AUXIN_VOL               0x16
#define AC97_IO_PCMOUT_VOL              0x18
#define AC97_IO_REC_SELECT              0x1A
#define AC97_IO_REC_GAIN                0x1C
#define AC97_IO_REC_GAIN_MIC            0x1E
#define AC97_IO_GENERAL                 0x20
#define AC97_IO_3D_CONTROL              0x22
#define AC97_IO_INT_PAGE                0x24
#define AC97_IO_POWERDOWN               0x26
#define AC97_IO_EXT_AUDIO               0x28
#define AC97_IO_EXT_AUDIO_CTRL          0x2A
#define AC97_IO_PCM_FRONT_DAC           0x2C
#define AC97_IO_PCM_SURR_DAC            0x2E
#define AC97_IO_PCM_LFE_DAC             0x30
#define AC97_IO_PCM_LR_ADC              0x32
#define AC97_IO_MIC_ADC                 0x34

#define AC97_IO_VENDOR_ID_1             0x7C
#define AC97_IO_VENDOR_ID_2             0x7E

#define MIXREG_PRIMARY_OFFSET           0x00
#define MIXREG_SECONDARY_OFFSET         0x80
#define MIXREG_TERTIARY_OFFSET          0x100

#define MIXREG_PRIMARY(r)               (MIXREG_PRIMARY_OFFSET + (r))
#define MIXREG_SECONDARY(r)             (MIXREG_SECONDARY_OFFSET + (r))
#define MIXREG_TERTIARY(r)              (MIXREG_TERTIARY_OFFSET + (r))

#define AC97_IO_PIBASE                  0x00
#define AC97_IO_POBASE                  0x10
#define AC97_IO_MCBASE                  0x20
#define AC97_IO_MC2BASE                 0x40
#define AC97_IO_PI2BASE                 0x50
#define AC97_IO_SPBASE                  0x60

#define AC97_IO_OFFSET_BDBAR            0x00
#define AC97_IO_OFFSET_CIV              0x04
#define AC97_IO_OFFSET_LVI              0x05
#define AC97_IO_OFFSET_SR               0x06
#define AC97_IO_OFFSET_PICB             0x08
#define AC97_IO_OFFSET_PIV              0x0A
#define AC97_IO_OFFSET_CR               0x0B

#define AC97_IO_BDBAR(base)             ((base) + AC97_IO_OFFSET_BDBAR)
#define AC97_IO_CIV(base)               ((base) + AC97_IO_OFFSET_CIV)
#define AC97_IO_LVI(base)               ((base) + AC97_IO_OFFSET_LVI)
#define AC97_IO_SR(base)                ((base) + AC97_IO_OFFSET_SR)
#define AC97_IO_PICB(base)              ((base) + AC97_IO_OFFSET_PICB)
#define AC97_IO_PIV(base)               ((base) + AC97_IO_OFFSET_PIV)
#define AC97_IO_CR(base)                ((base) + AC97_IO_OFFSET_CR)

#define AC97_IO_SR_FIFOE                0x0010
#define AC97_IO_SR_BCIS                 0x0008
#define AC97_IO_SR_LVBCI                0x0004
#define AC97_IO_SR_CELV                 0x0002
#define AC97_IO_SR_DCH                  0x0001

#define AC97_IO_CR_IOCE                 0x10
#define AC97_IO_CR_FEIE                 0x08
#define AC97_IO_CR_LVBIE                0x04
#define AC97_IO_CR_RR                   0x02
#define AC97_IO_CR_RPBM                 0x01

#define AC97_IO_GLOB_CNT                0x2C
#define AC97_IO_GLOB_STA                0x30
#define AC97_IO_CAS                     0x34
#define AC97_IO_SDM                     0x80

#endif // ARC_SERVICES_AUDIO_AC97_REGS_H

