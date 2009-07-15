#ifndef ARC_SERVICES_AUDIO_AC97_REGS_H
#define ARC_SERVICES_AUDIO_AC97_REGS_H

#define AMREG_RESET             0x00
#define AMREG_MASTER_VOL        0x02
#define AMREG_AUXOUT_VOL        0x04
#define AMREG_MONO_VOL          0x06
#define AMREG_MASTER_TONE       0x08
#define AMREG_PCBEEP_VOL        0x0A
#define AMREG_PHONE_VOL         0x0C
#define AMREG_MIC_VOL           0x0E
#define AMREG_LINEIN_VOL        0x10
#define AMREG_CD_VOL            0x12
#define AMREG_VIDEO_VOL         0x14
#define AMREG_AUXIN_VOL         0x16
#define AMREG_PCMOUT_VOL        0x18
#define AMREG_REC_SELECT        0x1A
#define AMREG_REC_GAIN          0x1C
#define AMREG_REC_GAIN_MIC      0x1E
#define AMREG_GENERAL           0x20
#define AMREG_3D_CONTROL        0x22
#define AMREG_INT_PAGE          0x24
#define AMREG_POWERDOWN         0x26
#define AMREG_EXT_AUDIO         0x28
#define AMREG_EXT_AUDIO_CTRL    0x2A
#define AMREG_PCM_FRONT_DAC     0x2C
#define AMREG_PCM_SURR_DAC      0x2E
#define AMREG_PCM_LFE_DAC       0x30
#define AMREG_PCM_LR_ADC        0x32
#define AMREG_MIC_ADC           0x34

#define AMREG_VENDOR_ID_1       0x7C
#define AMREG_VENDOR_ID_2       0x7E

#define MIXREG_PRIMARY_OFFSET   0x00
#define MIXREG_SECONDARY_OFFSET 0x80
#define MIXREG_TERTIARY_OFFSET  0x100

#define MIXREG_PRIMARY(r)       (MIXREG_PRIMARY_OFFSET + (r))
#define MIXREG_SECONDARY(r)     (MIXREG_SECONDARY_OFFSET + (r))
#define MIXREG_TERTIARY(r)      (MIXREG_TERTIARY_OFFSET + (r))

#endif // ARC_SERVICES_AUDIO_AC97_REGS_H

