/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///
/// @file   Interfaces/arc/chipset.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// Reference:   Intel I/O Controller Hub 7 (ICH7) Family Data Sheet,
//              January 2006.

//$Id: chipset.h 392 2008-09-02 11:49:51Z hro $

#ifndef ARC_IA32_DEVICE_ICH7_H
#define ARC_IA32_DEVICE_ICH7_H

//
// PCI Configuration Space
//

//
// I/O Mapped Registers
// @see AP440FX mother board, PIIX4, ICH7 datasheet
//
#define IO_DMA              0x0000
#define IO_PCI_CONFADD      0x0CF8
#define IO_PCI_CONFDATA     0x0CFC

#define IO_PIIX_PIDE_CMD    0x01F0      // Primary command block
#define IO_PIIX_SIDE_CMD    0x0170      // Secondary command block
#define IO_PIIX3_PIDE_CTRL  0x03F6      // Primary control block
#define IO_PIIX3_SIDE_CTRL  0x0376      // Secondary control block
#define IO_PIIX4_PIDE_CTRL  0x03F4      // Primary control block
#define IO_PIIX4_SIDE_CTRL  0x0374      // Secondary command block

#define IO_SIDE_STS         0x0377
#define IO_PIDE_STS         0x03F7

//
// Configuration Address Register Map (0x0CF8)
//
#define CONFADD_CFGE                0x80000000
#define CONFADD_RESERVED1           0x7F000000
#define CONFADD_BN                  0x00FF0000
#define CONFADD_DN                  0x0000F800
#define CONFADD_FN                  0x00000700
#define CONFADD_REGNO               0x000000FC
#define CONFADD_RESERVED2           0x00000003

#define CONFADD_BN_BITS             16          // Bus number
#define CONFADD_DN_BITS             11          // Device number
#define CONFADD_FN_BITS             8           // Function number

#define CONFADD(bn, dn, fn)     \
    (((bn) << CONFADD_BN_BITS) | ((dn) << CONFADD_DN_BITS) | \
    ((fn) << CONFADD_FN_BITS))

#define HOST_BRIDGE                 CONFADD(0, 0, 0)

//
// PIIX3 PCI Configuration Space Map
//
#define PIIX3_BRIDGE                CONFADD(0, 7, 0)
#define PIIX3_IDE                   CONFADD(0, 7, 1)
#define PIIX3_USB                   CONFADD(0, 7, 2)
#define I82557_ETH                  CONFADD(0, 6, 0)

//
// PIIX4 PCI Configuration Space Map
//
// PCI/AGP bridge
#define PIIX4_AGP                   CONFADD(0, 1, 0)
// PCI/ISA bridge
#define PIIX4_BRIDGE                CONFADD(0, 7, 0)
// IDE bus master
#define PIIX4_IDE                   CONFADD(0, 7, 1)
// USB
#define PIIX4_USB                   CONFADD(0, 7, 2)
// Power management
#define PIIX4_PM                    CONFADD(0, 7, 3)
// Ethernet
#define I82558_ETH                  CONFADD(0, 6, 0)

//
// ICH6/7 PCI Configuration Space Map
//
#define ICH_BRIDGE                  CONFADD(0, 30, 0)
#define ICH_AUDIO                   CONFADD(0, 30, 2)
#define ICH_MODEM                   CONFADD(0, 30, 3)

#define ICH_LPC                     CONFADD(0, 31, 0)
#define ICH_IDE                     CONFADD(0, 31, 1)
#define ICH_SATA                    CONFADD(0, 31, 2)
#define ICH_SMBUS                   CONFADD(0, 31, 3)

#define ICH_UHCI1                   CONFADD(0, 29, 0)
#define ICH_UHCI2                   CONFADD(0, 29, 1)
#define ICH_UHCI3                   CONFADD(0, 29, 2)
#define ICH_UHCI4                   CONFADD(0, 29, 3)
#define ICH_EHCI                    CONFADD(0, 29, 7)

#define ICH_PCIX1                   CONFADD(0, 28, 0)
#define ICH_PCIX2                   CONFADD(0, 28, 1)
#define ICH_PCIX3                   CONFADD(0, 28, 2)
#define ICH_PCIX4                   CONFADD(0, 28, 3)
#define ICH_PCIX5                   CONFADD(0, 28, 4)
#define ICH_PCIX6                   CONFADD(0, 28, 5)

#define ICH_HDAC                    CONFADD(0, 27, 0)

#define ICH_LAN                     CONFADD(1, 8, 0)

//
// PCI Register Addresses
// (Appears in the PCI configuration data register)
//
#define PCI_VID                     0x00    // Vendor Identification
#define PCI_DID                     0x02    // Device Identification
#define PCI_PCICMD                  0x04    // PCI Command
#define PCI_PCISTS                  0x06    // PCI Status
#define PCI_RID                     0x08    // Revision Identification
#define PCI_CC                      0x09    // Class code
#define PCI_PI                      0x09    // Programming Interface
#define PCI_SCC                     0x0A    // Sub Class Code
#define PCI_BCC                     0x0B    // Base Class Code
#define PCI_PMLT                    0x0D    // Primary Master Latency Timer
#define PCI_HDR                     0x0E    // Header Type
#define PCI_BIST                    0x0F    // BIST
#define PCI_PCMD_BAR                0x10    // Primary Command Block Base Addr
#define PCI_PCNL_BAR                0x14    // Primary Control Block Base Addr
#define PCI_SCMD_BAR                0x18    // Secondary Command Block Base 
#define PCI_SCNL_BAR                0x1C    // Secondary Control Block Base
#define PCI_BAR                     0x20    // Legacy Bus Master Base Address
#define PCI_ABAR                    0x24    // AHCI Base Address
#define PCI_SVID                    0x2C    // Subsystem Vendor Identification
#define PCI_SID                     0x2E    // Subsystem Identification
#define PCI_CAP                     0x34    // Capabilities Pointer
#define PCI_INT_LN                  0x3C    // Interrupt Line
#define PCI_INT_PN                  0x3D    // Interrupt Pin

#define PCI_EPBAR                   0x40    // Egress port base address
#define PCI_PIDE_TIMP               0x40    // Primary IDE Timing
#define PCI_PIDE_TIMS               0x42    // Secondary IDE Timing
#define PCI_MCHBAR                  0x44    // MCH memory mapped register base
#define PCI_SIDE_TIM                0x44    // Slave IDE Timing
#define PCI_PCIEXBAR                0x48    // PCI Express register base
#define PCI_SDMA_CNT                0x48    // Synchronous DMA Control
#define PCI_SDMA_TIM                0x4A    // Synchronous DMA Timing
#define PCI_DMIBAR                  0x4C    // Root complex register base

#define PCI_IDE_CONFIG              0x54    // IDE I/O Configuration
#define PCI_SATA_PID                0x70
#define PCI_SATA_PC                 0x72
#define PCI_SATA_PMCS               0x74
#define PCI_SATA_MSICI              0x80
#define PCI_SATA_MSIMC              0x82
#define PCI_SATA_MSIMA              0x84
#define PCI_SATA_MSIMD              0x88
#define PCI_SATA_MAP                0x90
#define PCI_PCS                     0x92    // SATA port control and status
#define PCI_SATA_SIR                0x94
#define PCI_LAC                     0x97    // Legacy access control
#define PCI_REMAPBASE               0x98    // Remap base address
#define PCI_REMAPLIMIT              0x9A    // Remap limit address
#define PCI_TOLUD                   0x9C    // Top of low usable DRAM
#define PCI_TOM                     0xA0    // Top of memory
#define PCI_SATA_SIRI               0xA0
#define PCI_SATA_STRD               0xA4
#define PCI_SATA_SCAP0              0xA8
#define PCI_SATA_SCAP1              0xAC
#define PCI_ATC                     0xC0    // APM Trapping Control
#define PCI_ATS                     0xC4    // ATM Trapping Status
#define PCI_SATA_SP                 0xD0
#define PCI_SATA_BFCS               0xE0
#define PCI_SATA_BFTD1              0xE4
#define PCI_SATA_BFTD2              0xE8

//
// PCI Register Values and Masks
//

// Vender Identification Register
#define PCI_VID_INTEL                   0x8086  // Default value

// Device Identification Register
#define PCI_DID_INTEL_440FX             0x1237
#define PCI_DID_INTEL_440BX_AGP         0x7190
#define PCI_DID_INTEL_440BX             0x7192
#define PCI_DID_INTEL_915               0x2580
#define PCI_DID_INTEL_945               0x2770
#define PCI_DID_INTEL_965               0x2980
#define PCI_DID_INTEL_975X              0x277C

// PCI Command Register Mask
#define PCI_PCICMD_ID               0x0400
#define PCI_PCICMD_FBE              0x0200
#define PCI_PCICMD_SERR             0x0100
#define PCI_PCICMD_WCC              0x0080
#define PCI_PCICMD_PERR             0x0040      // Parity error
#define PCI_PCICMD_VPS              0x0020
#define PCI_PCICMD_PMWE             0x0010
#define PCI_PCICMD_SCE              0x0008
#define PCI_PCICMD_BME              0x0004
#define PCI_PCICMD_MSE              0x0002
#define PCI_PCICMD_IOSE             0x0001

// PCI Status Register
#define PCI_PCISTS_DPE              0x8000      // Detected Parity Error
#define PCI_PCISTS_SSE              0x4000      // Signaled System Error
#define PCI_PCISTS_RMA              0x2000      // Received Master Abort
#define PCI_PCISTS_RTA              0x1000      // Received Target Abort
#define PCI_PCISTS_STA              0x0800      // Signaled Target Abort
#define PCI_PCISTS_DEV              0x0600
#define PCI_PCISTS_DPED             0x0100      // Data Prity Error Detected
#define PCI_PCISTS_FB2BC            0x0080      // Fast Back to Back Capable
#define PCI_PCISTS_UDF              0x0040      // User Defined Features
#define PCI_PCISTS_66MHZ            0x0020      // 66MHz Capable
#define PCI_PCISTS_CLIST            0x0010      // Capabilities List
#define PCI_PCISTS_INTS             0x0008      // Interrupt Status
#define PCI_PCISTS_RESERVED         0x0007

// Revision Identification Register
#define PCI_RID_

// Programming Interface Register
#define PCI_PI_BMSUPPORT            0x0080
#define PCI_PI_RESERVED             0x0070
#define PCI_PI_SOP_MODE_CAP         0x0008
#define PCI_PI_SOP_MODE_SEL         0x0004
#define PCI_PI_POP_MODE_CAP         0x0002
#define PCI_PI_POP_MODE_SEL         0x0001

// Sub Class Code Register
// TODO
#define PCI_SCC_ICH7_DESKTOP        0x01
#define PCI_SCC_ICH7_M              0x01
#define PCI_SCC_ICH7_M_DH

// Primary Command Block Base Address Register
#define PCI_PCMD_BAR_RESERVED1      0xFFFF0000
#define PCI_PCMD_BAR_BASE_ADDR      0x0000FFF8
#define PCI_PCMD_BAR_RESERVED2      0x00000006
#define PCI_PCMD_BAR_RTE            0x00000001

// Primary Control Block Base Address Register
#define PCI_PCNL_BAR_RESERVED1      0xFFFF0000
#define PCI_PCNL_BAR_BASE_ADDR      0x0000FFFC
#define PCI_PCNL_BAR_RESERVED2      0x00000002
#define PCI_PCNL_BAR_RTE            0x00000001

// Secondary Command Block Base Address Register
#define PCI_SCMD_BAR_RESERVED1      0xFFFF0000
#define PCI_SCMD_BAR_BASE_ADDR      0x0000FFF8
#define PCI_SCMD_BAR_RESERVED2      0x00000006
#define PCI_SCMD_BAR_RTE            0x00000001

// Secondary Control Block Base Address Register
#define PCI_SCNL_BAR_RESERVED1      0xFFFF0000
#define PCI_SCNL_BAR_BASE_ADDR      0x0000FFFC
#define PCI_SCNL_BAR_RESERVED2      0x00000002
#define PCI_SCNL_BAR_RTE            0x00000001

// Legacy Bus Master Base Address Register
#define PCI_BAR_RESERVED1           0xFFFF0000
#define PCI_BAR_BASE_ADDR           0x0000FFF0
#define PCI_BAR_RESERVED2           0x0000000E
#define PCI_BAR_RTE                 0x00000001

#define PCI_BM_BASE_RESERVED1       0xFFFF0000
#define PCI_BM_BASE_BASE_ADDR       0x0000FFF0
#define PCI_BM_BASE_RESERVED2       0x0000000E
#define PCI_BM_BASE_RTE             0x00000001

// AHCE Base Address Register
#define PCI_ABAR_BASE_ADDR          0xFFFFFC00
#define PCI_ABAR_RESERVED           0x000003F0
#define PCI_ABAR_PF                 0x00000008
#define PCI_ABAR_TP                 0x00000006
#define PCI_ABAR_RTE                0x00000001

// Primary IDE Timing Register
#define PCI_IDE_TIMP_IDE            0x8000
#define PCI_IDE_TIMP_SITRE          0x4000
#define PCI_IDE_TIMP_ISP            0x3000
#define PCI_IDE_TIMP_RESERVED       0x0C00
#define PCI_IDE_TIMP_RCT            0x0300
#define PCI_IDE_TIMP_DTE1           0x0080
#define PCI_IDE_TIMP_PPE1           0x0040
#define PCI_IDE_TIMP_IE1            0x0020
#define PCI_IDE_TIMP_TIME1          0x0010
#define PCI_IDE_TIMP_DTE0           0x0008
#define PCI_IDE_TIMP_PPE0           0x0004
#define PCI_IDE_TIMP_IE0            0x0002
#define PCI_IDE_TIMP_TIME0          0x0001

// Secondary IDE Timing Register
#define PCI_IDE_TIMS_IDE            0x8000
#define PCI_IDE_TIMS_NOP1           0x7000
#define PCI_IDE_TIMS_RESERVED       0x0800
#define PCI_IDE_TIMS_NOP2           0x07FF

// Slave IDE Timing Register (0x44)
#define PCI_SLV_IDETIM_SISP1        0xC0
#define PCI_SLV_IDETIM_SRCT1        0x30
#define PCI_SLV_IDETIM_PISP1        0x0C
#define PCI_SLV_IDETIM_PRCT1        0x03

// Synchronous DMA Control Register (0x48)
#define PCI_SDMA_CNT_RESERVED       0xF0
#define PCI_SDMA_CNT_NOP            0x0C
#define PCI_SDMA_CNT_PSDE1          0x02
#define PCI_SDMA_CNT_PSDE0          0x01

// Synchronous DMA Timing Register (0x4A - 0x4B)
#define PCI_SDMA_TIM_RESERVED1      0xC000
#define PCI_SDMA_TIM_SCT1           0x3000
#define PCI_SDMA_TIM_RESERVED2      0x0C00
#define PCI_SDMA_TIM_SCT0           0x0300
#define PCI_SDMA_TIM_RESERVED3      0x00C0
#define PCI_SDMA_TIM_PCT1           0x0030
#define PCI_SDMA_TIM_RESERVED4      0x000C
#define PCI_SDMA_TIM_PCT0           0x0003

// IDE I/O Configuration Register (0x54 - 0x57)
#define PCI_IDE_CONFIG_RESERVED1        0xFF000000
#define PCI_IDE_CONFIG_MS               0x00F00000
#define PCI_IDE_CONFIG_SEC_SIG_MODE     0x000C0000
#define PCI_IDE_CONFIG_PRIM_SIG_MODE    0x00030000
#define PCI_IDE_CONFIG_SIG_MODE         0x00030000
#define PCI_IDE_CONFIG_FAST_SCB1        0x00008000
#define PCI_IDE_CONFIG_FAST_SCB0        0x00004000
#define PCI_IDE_CONFIG_FAST_PCB1        0x00002000
#define PCI_IDE_CONFIG_FAST_PCB0        0x00001000
#define PCI_IDE_CONFIG_RESERVED2        0x00000F00
#define PCI_IDE_CONFIG_NOP1             0x00000080
#define PCI_IDE_CONFIG_NOP2             0x00000040
#define PCI_IDE_CONFIG_PSCCR            0x00000020
#define PCI_IDE_CONFIG_PMCCR            0x00000010
#define PCI_IDE_CONFIG_SCB1             0x00000008
#define PCI_IDE_CONFIG_SCB0             0x00000004
#define PCI_IDE_CONFIG_PCB1             0x00000002
#define PCI_IDE_CONFIG_PCB0             0x00000001

// PCI Power Management Capability Identification Register (0x70 - 0x71)
#define PCI_PID_NEXT                    0xFF00
#define PCI_PID_CID                     0x00FF

// PCI Power Management Capabilities Register (0x72 - 0x73)
#define PCI_PC_PME_SUP                  0xF800
#define PCI_PC_D2_SUP                   0x0400
#define PCI_PC_D1_SUP                   0x0200
#define PCI_PC_AUX_CUR                  0x01C0
#define PCI_PC_DSI                      0x0020
#define PCI_PC_RESERVED                 0x0010
#define PCI_PC_PME_CLK                  0x0008
#define PCI_PC_VER                      0x0007

// PCI Power Management Control and Status Register (0x74 - 0x75)
#define PCI_PMCS_PMES                   0x8000
#define PCI_PMCS_RESERVED1              0x7E00
#define PCI_PMCS_PMEE                   0x0100
#define PCI_PMCS_RESERVED2              0x00FC
#define PCI_PMCS_PS                     0x0003

// Message Signaled Interrupt Capability (0x80 - 0x81)
#define PCI_MSICI_NEXT                  0xFF00
#define PCI_MSICI_CID                   0x00FF

// Message Signaled Interrupt Message Control (0x82 - 0x83)
#define PCI_MSIMC_RESERVED              0xFF00
#define PCI_MSIMC_C64                   0x0080
#define PCI_MSIMC_MME                   0x0070
#define PCI_MSIMC_MMC                   0x000E
#define PCI_MSIMC_MSIE                  0x0001

// Message Signaled Interrupt Message Address (0x84 - 0x87)
#define PCI_MSIMA_ADDR                  0xFFFFFFFC
#define PCI_MSIMA_RESERVED              0x00000003

// Address Map Register (0x90)
#define PCI_MAP_SMS                     0xC0
#define PCI_MAP_SMS_IDE                 (0 << 6)
#define PCI_MAP_SMS_AHCI                (1 << 6)
#define PCI_MAP_SMS_RAID                (2 << 6)
#define PCI_MAP_RESERVED                0x3C
#define PCI_MAP_MV                      0x03
#define PCI_MAP_MV_NONCOMBINED          0
#define PCI_MAP_MV_COMBINED_IDE         1
#define PCI_MAP_MV_COMBINED_SATA        2

// Port Control and Status Register (0x92 - 0x93)
#define PCI_PCS_RESERVED                0xFF00
#define PCI_PCS_P3P                     0x0080
#define PCI_PCS_P2P                     0x0040
#define PCI_PCS_P1P                     0x0020
#define PCI_PCS_P0P                     0x0010
#define PCI_PCS_P3E                     0x0008
#define PCI_PCS_P2E                     0x0004
#define PCI_PCS_P1E                     0x0002
#define PCI_PCS_P0E                     0x0001

// SATA Initialization Register (0x94 - 0x97)
#define PCI_SIR_RESERVED1               0x80000000
#define PCI_SIR_SCRD                    0x40000000
#define PCI_SIR_RESERVED2               0x20000000
#define PCI_SIR_SCRE                    0x10000000
#define PCI_SIR_SIF3                    0x0F000000
#define PCI_SIR_SIF2                    0x00800000
#define PCI_SIR_RESERVED3               0x007FFC00
#define PCI_SIR_SCRAE                   0x00000200
#define PCI_SIR_SIF1                    0x000001FF

// SATA Indexed Registers Index
#define PCI_SIRI_IDX                    0xFC
#define PCI_SIRI_RESERVED               0x03

// SATA Capability Register 0 (0xA8 - 0xAB)
#define PCI_SCAP0_RESERVED              0xFF000000
#define PCI_SCAP0_MAJREV                0x00F00000
#define PCI_SCAP0_JINREV                0x000F0000
#define PCI_SCAP0_NEXT                  0x0000FF00
#define PCI_SCAP0_CAP                   0x000000FF

// SATA Capability Register 1 (0xAC - 0xAF)
#define PCI_SCAP1_RESERVED              0xFFFF0000
#define PCI_SCAP1_BAROFST               0x0000FFF0
#define PCI_SCAP1_BARLOC                0x0000000F

// APM Trapping Control Register (0xC0)
#define PCI_ATC_RESERVED                0xF0
#define PCI_ATC_SST                     0x08
#define PCI_ATC_SMT                     0x04
#define PCI_ATC_PST                     0x02
#define PCI_ATC_PMT                     0x01

// APM Trapping Status Register (0xC4)
#define PCI_ATS_RESERVED                0xF0
#define PCI_ATS_SST                     0x08
#define PCI_ATS_SPT                     0x04
#define PCI_ATS_PST                     0x02
#define PCI_ATS_PMT                     0x01

// BIST FIS Control/Status Register (0xE0 - 0xE3)
#define PCI_BFCS_RESERVED1              0xFFFFC000
#define PCI_BFCS_P3BFI                  0x00002000
#define PCI_BFCS_P2BFI                  0x00001000
#define PCI_BFCS_BFS                    0x00000800
#define PCI_BFCS_BFF                    0x00000400
#define PCI_BFCS_P1BFI                  0x00000200
#define PCI_BFCS_P0BFI                  0x00000100
#define PCI_BFCS_PARAM                  0x000000FC
#define PCI_BFCS_RESERVED2              0x00000003

//
// Bus Master IDE I/O Registers
//
#define BAR_BMICP                       0x00
#define BAR_BMISP                       0x02
#define BAR_BMIDP                       0x04
#define BAR_BMICS                       0x08
#define BAR_BMISS                       0x0A
#define BAR_BMIDS                       0x0C
#define BAR_AIR                         0x10
#define BAR_AIDR                        0x14

// Bus Master IDE Command Register (0x00)
#define BAR_BMICP_RESERVED1             0xF0
#define BAR_BMICP_RWC                   0x08
#define BAR_BMICP_RESERVED2             0x06
#define BAR_BMICP_START                 0x01

// Bus Master IDE Command Register (0x08)
#define BAR_BMICS_RESERVED1             0xF0
#define BAR_BMICS_RWC                   0x08
#define BAR_BMICS_RESERVED2             0x06
#define BAR_BMICS_START                 0x01

// Bus Master IDE Status Register (0x02)
#define BAR_BMISP_PRDIS                 0x80
#define BAR_BMISP_CAP1                  0x40
#define BAR_BMISP_CAP0                  0x20
#define BAR_BMISP_RESERVED              0x18
#define BAR_BMISP_IDEIRQ                0x04
#define BAR_BMISP_ERROR                 0x02
#define BAR_BMISP_ACT                   0x01

// Bus Master IDE Status Register (0x0A)
#define BAR_BMISS_PRDIS                 0x80
#define BAR_BMISS_CAP1                  0x40
#define BAR_BMISS_CAP0                  0x20
#define BAR_BMISS_RESERVED              0x18
#define BAR_BMISS_IDEIRQ                0x04
#define BAR_BMISS_ERROR                 0x02
#define BAR_BMISS_ACT                   0x01

// Bus Master IDE Descriptor Table Pointer Register (0x04)
#define BAR_BMIDP_ADDR                  0xFFFFFFFC
#define BAR_BMIDP_RESERVED              0x00000003

// Bus Master IDE Descriptor Table Pointer Register (0x0C)
#define BAR_BMIDS_ADDR                  0xFFFFFFFC
#define BAR_BMIDS_RESERVED              0x00000003

// AHCI Index Register (0x10)
#define BAR_AIR_RESERVED1               0xFFFFFC00
#define BAR_AIR_INDEX                   0x000003FC
#define BAR_AIR_RESERVED2               0x00000003


//----------------------------------------------------------------------------
// AHCI Registers
//----------------------------------------------------------------------------

//
// AHCI Register Address Map
//
#define ABAR_GHCR                       0x0000
#define ABAR_P0PCR                      0x0100
#define ABAR_P1PCR                      0x0180
#define ABAR_P2PCR                      0x0200
#define ABAR_P3PCR                      0x0280

//
// AHCI Generic Host Control Registers Address Map
//
#define ABAR_CAP                        0x00
#define ABAR_GHC                        0x04
#define ABAR_IS                         0x08
#define ABAR_PI                         0x0C
#define ABAR_VS                         0x10

// Host Capabilities Register (0x00 - 0x03)
#define ABAR_CAP_S64A                   0x80000000
#define ABAR_CAP_SCQA                   0x40000000
#define ABAR_CAP_SSNTF                  0x20000000
#define ABAR_CAP_SIS                    0x10000000
#define ABAR_CAP_SSS                    0x08000000
#define ABAR_CAP_SALP                   0x04000000
#define ABAR_CAP_SAL                    0x02000000
#define ABAR_CAP_SCLO                   0x01000000
#define ABAR_CAP_ISS                    0x00F00000
#define ABAR_CAP_SNZO                   0x00080000
#define ABAR_CAP_SPSA                   0x00040000
#define ABAR_CAP_PMS                    0x00020000
#define ABAR_CAP_PMFS                   0x00010000
#define ABAR_CAP_PMD                    0x00008000
#define ABAR_CAP_SSC                    0x00004000
#define ABAR_CAP_PSC                    0x00002000
#define ABAR_CAP_NCS                    0x00001F00
#define ABAR_CAP_RESERVED               0x000000E0
#define ABAR_CAP_NPS                    0x0000001F

#define ABAR_CAP_NCS_DEFAULT            32
#define ABAR_CAP_NPS_DEFAULT            4

// Global ICH7 Control Register (0x04 - 0x07)
#define ABAR_GHC_AE                     0x80000000
#define ABAR_GHC_RESERVED               0x7FFFFFFC
#define ABAR_GHC_IE                     0x00000002
#define ABAR_GHC_HR                     0x00000001

// Interrupt Status Register (0x08 - 0x0B)
#define ABAR_IS_RESERVED                0xFFFFFFF0
#define ABAR_IS_IPS3                    0x00000008
#define ABAR_IS_IPS2                    0x00000004
#define ABAR_IS_IPS1                    0x00000002
#define ABAR_IS_IPS0                    0x00000001

// Ports Implemented Register (0x0C - 0x0F)
#define ABAR_PI_RESERVED                0xFFFFFFF0
#define ABAR_PI_PI3                     0x00000008
#define ABAR_PI_PI2                     0x00000004
#define ABAR_PI_PI1                     0x00000002
#define ABAR_PI_PI0                     0x00000001

// AHCI Version (0x10 - 0x13)
#define ABAR_VS_MJR                     0xFFFF0000
#define ABAR_VS_MNR                     0x0000FFFF

// Port Registers Offset
#define ABAR_PXCLB                      0x0000
#define ABAR_PXCLBU                     0x0004
#define ABAR_PXFB                       0x0008
#define ABAR_PXFBU                      0x000C
#define ABAR_PXIS                       0x0010
#define ABAR_PXIE                       0x0014
#define ABAR_PXCMD                      0x0018
#define ABAR_PXTFD                      0x0020
#define ABAR_PXSIG                      0x0024
#define ABAR_PXSSTS                     0x0028
#define ABAR_PXSCTL                     0x002C
#define ABAR_PXSERR                     0x0030
#define ABAR_PXSACT                     0x0034
#define ABAR_PXCI                       0x0038

// Command List Base Address Register (0x100, 0x180, 0x200, 0x280)
#define ABAR_PXCLB_MASK                 0xFFFFFC00
#define ABAR_PXCLB_RESERVED             0x00000300

// FIS Base Address Register (0x108, 0x188, 0x208, 0x288)
#define ABAR_PXFB_MASK                  0xFFFFFF00
#define ABAR_PXFB_RESERVED              0x000000FF

// FIS Base Address Uppport 32-bits Registers
#define ABAR_PXFBU_MASK                 0xFFFFFFF8
#define ABAR_PXFBU_RESERVED             0x00000007

// Interrupt Status Register
#define ABAR_PXIS_CPDS                  0x80000000
#define ABAR_PXIS_TFES                  0x40000000
#define ABAR_PXIS_HBFS                  0x20000000
#define ABAR_PXIS_HBDS                  0x10000000
#define ABAR_PXIS_IFS                   0x08000000
#define ABAR_PXIS_INFS                  0x04000000
#define ABAR_PXIS_RESERVED1             0x02000000
#define ABAR_PXIS_OFS                   0x01000000
#define ABAR_PXIS_IPMS                  0x00800000
#define ABAR_PXIS_PRCS                  0x00400000
#define ABAR_PXIS_RESERVED2             0x003FFF00
#define ABAR_PXIS_DIS                   0x00000080
#define ABAR_PXIS_PCS                   0x00000040
#define ABAR_PXIS_DPS                   0x00000020
#define ABAR_PXIS_UFS                   0x00000010
#define ABAR_PXIS_SDBS                  0x00000008
#define ABAR_PXIS_DSS                   0x00000004
#define ABAR_PXIS_PSS                   0x00000002
#define ABAR_PXIS_DHRS                  0x00000001

// Interrupt Enable Register (0x114, 0x194, 0x214, 0x294)
#define ABAR_PXIE_CPDE                  0x80000000
#define ABAR_PXIE_TFEE                  0x40000000
#define ABAR_PXIE_HBFE                  0x20000000
#define ABAR_PXIE_HBDE                  0x10000000
#define ABAR_PXIE_HBDE2                 0x08000000
#define ABAR_PXIE_INFE                  0x04000000
#define ABAR_PXIE_RESERVED1             0x02000000
#define ABAR_PXIE_OFE                   0x01000000
#define ABAR_PXIE_IPME                  0x00800000
#define ABAR_PXIE_PRCE                  0x00400000
#define ABAR_PXIE_RESERVED2             0x003FFF00
#define ABAR_PXIE_DIE                   0x00000080
#define ABAR_PXIE_PCE                   0x00000040
#define ABAR_PXIE_DPE                   0x00000020
#define ABAR_PXIE_UFIE                  0x00000010
#define ABAR_PXIE_SDBE                  0x00000008
#define ABAR_PXIE_DSE                   0x00000004
#define ABAR_PXIE_PSE                   0x00000002
#define ABAR_PXIE_DHRE                  0x00000001

// Command Register (0x118, 0x198, 0x218, 0x298)
#define ABAR_PXCMD_ICC_MASK             0xF0000000
#define ABAR_PXCMD_ICC_NOP              0x00000000
#define ABAR_PXCMD_ICC_ACTIVE           0x10000000
#define ABAR_PXCMD_ICC_PARTIAL          0x20000000
#define ABAR_PXCMD_ICC_SLUMBER          0x60000000
#define ABAR_PXCMD_ASP                  0x08000000
#define ABAR_PXCMD_ALPE                 0x04000000
#define ABAR_PXCMD_DLAE                 0x02000000
#define ABAR_PXCMD_ATAPI                0x01000000
#define ABAR_PXCMD_RESERVED1            0x00F00000
#define ABAR_PXCMD_ISP                  0x00080000
#define ABAR_PXCMD_HPCP                 0x00040000
#define ABAR_PXCMD_PMA                  0x00020000
#define ABAR_PXCMD_PMFSE                0x00010000
#define ABAR_PXCMD_CR                   0x00008000
#define ABAR_PXCMD_FR                   0x00004000
#define ABAR_PXCMD_ISS                  0x00002000
#define ABAR_PXCMD_CCS                  0x00001F00
#define ABAR_PXCMD_RESERVED2            0x000000E0
#define ABAR_PXCMD_FRE                  0x00000010
#define ABAR_PXCMD_CLO                  0x00000008
#define ABAR_PXCMD_POD                  0x00000004
#define ABAR_PXCMD_SUD                  0x00000002
#define ABAR_PXCMD_ST                   0x00000001

// Task File Data Register (0x120, 0x1A0, 0x220, 0x2A0)
#define ABAR_PXTFD_RESERVED             0xFFFF0000
#define ABAR_PXTFD_ERR                  0x0000FF00
#define ABAR_PXTFD_STS_MASK             0x000000FF
#define ABAR_PXTFD_STS_BSY              0x00000080
#define ABAR_PXTFD_STS_DRQ              0x00000008
#define ABAR_PXTFD_STS_ERR              0x00000001

// Serial ATA Status Register (0x128, 0x1A8, 0x228, 0x2A8)
#define ABAR_PXSSTS_RESERVED            0xFFFFF000
#define ABAR_PXSSTS_IPM                 0x00000F00
#define ABAR_PXSSTS_SPD                 0x000000F0
#define ABAR_PXSSTS_DET_MASK            0x0000000F
#define ABAR_PXSSTS_DET_NODEV           0x00000000
#define ABAR_PXSSTS_DET_DEV             0x00000001
#define ABAR_PXSSTS_DET_COMM            0x00000003
#define ABAR_PXSSTS_DET_OFFLINE         0x00000004

// Serial ATA Control Register (0x12C, 0x1AC, 0x22C, 0x2AC)
#define ABAR_PXSCTL_RESERVED            0xFFF00000
#define ABAR_PXSCTL_PMP                 0x000F0000
#define ABAR_PXSCTL_SPM                 0x0000F000
#define ABAR_PXSCTL_IPM                 0x00000F00
#define ABAR_PXSCTL_SPD                 0x000000F0
#define ABAR_PXSCTL_DET_MASK            0x0000000F
#define ABAR_PXSCTL_DET_COMM            0x00000001
#define ABAR_PXSCTL_DET_DOWN            0x00000004

// Serial ATA Error Register (0x130, 0x1B0, 0x230, 0x2B0)
#define ABAR_PXSERR_DIAG                0xFFFF0000
#define ABAR_PXSERR_ERR                 0x0000FFFF

//
// System Address Map
//
enum {
    DOS_AREA_START =                    0x00000000,
    LEGACY_VEDEO_START =                0x000A0000,
    EXPANSION_AREA_START =              0x000C0000,
    SYSTEM_BIOS_LO_START =              0x000E0000,
    SYSTEM_BIOS_HI_START =              0x000F0000,
    MAIN_MEMORY_START =                 0x00100000,
    IOAPIC_CONFIG_START =               0xFEC00000,
    APIC_CONFIG_START =                 0xFEC80000,
    FSB_INTERRUPTS_START =              0xFEE00000,
    FSB_INTERRUPTS_END =                0xFEF00000,
    HIGH_BIOS_START =                   0xFFE00000,
};

#endif // ARC_IA32_DEVICE_ICH7_H

