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

#define MIXREG_PRIMARY(r)       (MIXREG_PRIMARY_OFFSET + (r))
#define MIXREG_SECONDARY(r)     (MIXREG_SECONDARY_OFFSET + (r))
#define MIXREG_TERTIARY(r)      (MIXREG_TERTIARY_OFFSET + (r))


#define AC97_IO_PIBDBAR                 0x00
#define AC97_IO_POBDBAR                 0x10
#define AC97_IO_MCBDBAR                 0x20
#define AC97_IO_MC2BDBAR                0x40
#define AC97_IO_PI2BDBAR                0x50
#define AC97_IO_SPBDBAR                 0x60
#define AC97_IO_BDBAR_MASK              0xFFFFFFFC

#define AC97_IO_PICIV                   0x04
#define AC97_IO_POCIV                   0x14
#define AC97_IO_MCCIV                   0x24
#define AC97_IO_MC2CIV                  0x44
#define AC97_IO_PI2CIV                  0x54
#define AC97_IO_SPCIV                   0x64

#define AC97_IO_PILVI                   0x05
#define AC97_IO_POLVI                   0x15
#define AC97_IO_MCLVI                   0x25
#define AC97_IO_MC2LVI                  0x45
#define AC97_IO_PI2LVI                  0x55
#define AC97_IO_SPLVI                   0x65

#define AC97_IO_PISR                    0x06
#define AC97_IO_POSR                    0x16
#define AC97_IO_MCSR                    0x26
#define AC97_IO_MC2SR                   0x46
#define AC97_IO_PI2SR                   0x56
#define AC97_IO_SPSR                    0x66

#define AC97_IO_SR_FIFOE                0x0010
#define AC97_IO_SR_BCIS                 0x0008
#define AC97_IO_SR_LVBCI                0x0004
#define AC97_IO_SR_CELV                 0x0002
#define AC97_IO_SR_DCH                  0x0001

#define AC97_IO_PIPICB                  0x08
#define AC97_IO_POPICB                  0x18
#define AC97_IO_MCPICB                  0x28
#define AC97_IO_MC2PICB                 0x48
#define AC97_IO_PI2PICB                 0x58
#define AC97_IO_SPPICB                  0x68

#define AC97_IO_PIPIV                   0x0A
#define AC97_IO_POPIV                   0x1A
#define AC97_IO_MCPIV                   0x2A
#define AC97_IO_MC2PIV                  0x4A
#define AC97_IO_PI2PIV                  0x5A
#define AC97_IO_SPPIV                   0x6A

#define AC97_IO_PICR                    0x0B
#define AC97_IO_POCR                    0x1B
#define AC97_IO_MCCR                    0x2B
#define AC97_IO_MC2CR                   0x4B
#define AC97_IO_PI2CR                   0x5B
#define AC97_IO_SRCR                    0x6B

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

