/*
 *
 *  Copyright (C) 2007, Waseda University. All rights reserved.
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
/// @file   Services/Devices/Sata/ATA.h
/// @brief  ATA header file
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.ac.jp>
/// @date   2006
///
// $Id: Ata.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEVICE_SATA_ATA_H
#define ARC_DEVICE_SATA_ATA_H

#include <arc/types.h>

#define ATA_SECTOR_SIZE		0x200

//---------------------------------------------------------------------------
//	ATA commands
//---------------------------------------------------------------------------

#define ATA_CMD_NOP		0x00	// Nop
#define ATA_CMD_RECALIBRATE	0x10	// Recalibrate
#define ATA_CMD_READ_SECTOR_R	0x20	// Read sector(s) with retries
#define ATA_CMD_READ_SECTOR	0x21	// Read sector(s) without retries
#define ATA_CMD_READ_LONG_R	0x22	// Read long with retries
#define ATA_CMD_READ_LONG	0x23	// Read long without retries
#define ATA_CMD_WRITE_SECTOR_R	0x30	// Write sector(s) with retries
#define ATA_CMD_WRITE_SECTOR	0x31	// Write sector(s) without retries
#define ATA_CMD_WRITE_LONG_R	0x32	// Write Long with retries
#define ATA_CMD_WRITE_LONG	0x33	// Write Long without retries
#define ATA_CMD_WRITE_VERITY	0x3C	// Write verify
#define ATA_CMD_READ_VERIFY_R	0x40	// Read verify sector(s) w/retries
#define ATA_CMD_READ_VERIFY	0x41	// Read verify sector(s) w/o retries
#define ATA_CMD_FORMAT		0x50	// Format track
#define ATA_CMD_SEEK		0x70	// Seek
#define ATA_CMD_DIAGNOSTIC	0x90	// Execute device diagnostic
#define ATA_CMD_INIT_PARM	0x91	// Initialize device parameters
#define ATA_CMD_MICROCODE	0x92	// Download microcode
#define ATA_CMD_STANDBY_IM_1	0x94	// Standby immediate
#define ATA_CMD_IDLE_IM_1 	0x95	// Idle immediate
#define ATA_CMD_STANDBY_1	0x96	// Standby
#define ATA_CMD_IDLE_1		0x97	// Idle
#define ATA_CMD_CHKPWR_1	0x98	// Check power mode
#define ATA_CMD_SLEEP_1		0x99	// Sleep
#define ATA_CMD_PACKET          0xA0    // Packet
#define ATA_CMD_IDENTIFY_PACKET 0xA1    // Identify packet device
#define ATA_CMD_SMART		0xB0	// SMART operations
#define ATA_CMD_READ_MULTI	0xC4	// Read multiple
#define ATA_CMD_WRITE_MULTI	0xC5	// Write multiple
#define ATA_CMD_MULTIMODE	0xC6	// Set multiple mode
#define ATA_CMD_READ_DMA_R	0xC8	// Read DMA with retries
#define ATA_CMD_READ_DMA	0xC9	// Read DMA without retries
#define ATA_CMD_WRITE_DMA_R	0xCA	// Write DMA with retries
#define ATA_CMD_WRITE_DMA	0xCB	// Write DMA without retries
#define ATA_CMD_DOORLOCK	0xDE	// Door lock
#define ATA_CMD_DOORUNLOCK	0xDF	// Door unlock
#define ATA_CMD_STANDBY_IM_2	0xE0	// Standby immediate
#define ATA_CMD_IDLE_IM_2	0xE1	// Idle immediate
#define ATA_CMD_STANDBY_2	0xE2	// Standby
#define ATA_CMD_IDLE_2		0xE3	// Idle
#define ATA_CMD_READ		0xE4	// Read buffer
#define ATA_CMD_CHKPWR_2	0xE5	// Check power mode
#define ATA_CMD_SLEEP_2		0xE6	// Sleep
#define ATA_CMD_IDENTIFY	0xEC	// Identify device
#define ATA_CMD_EJECT		0xED	// Media eject
#define ATA_CMD_IDENTIFY_DMA	0xEE	// Identify device DMA
#define ATA_CMD_FEATURES	0xEF	// Set features
#define ATA_CMD_SET_PASSWD	0xF1	// Security set password
#define ATA_CMD_UNLOCK		0xF2	// Security unlock
#define ATA_CMD_ERASE_PREP	0xF3	// Security erase prepare
#define ATA_CMD_ERASE_UNIT	0xF4	// Security erase unit
#define ATA_CMD_FREEZE		0xF5	// Security freeze lock
#define ATA_CMD_UNSET_PASSWD	0xF6	// Security disable password

//---------------------------------------------------------------------------
//	Parameters for Device Control register
//---------------------------------------------------------------------------

#define ATA_REG_DC_HOB          (1 << 7)    // High order byte
#define ATA_REG_DC_SRST		(1 << 2)    // Software reset
#define ATA_REG_DC_NLEN		(1 << 1)    // Enable the device interrupt

//---------------------------------------------------------------------------
//	Parameters for Device/Head register
//---------------------------------------------------------------------------

#define ATA_REG_DH_LBA		0x40	// LBA is enable if this bit is set, 
// otherwise CHS
#define ATA_REG_DH_DEV		0x10	// Device-1 is selected when this bit
// set, othersise Device-0

//---------------------------------------------------------------------------
//	Parameters for Error register
//---------------------------------------------------------------------------

#define ATA_REG_ERR_UNC		0x40	// Uncorrectable data error
#define ATA_REG_ERR_MC		0x20	// Media changed
#define ATA_REG_ERR_NID		0x10	// ID not found
#define ATA_REG_ERR_MCR		0x08	// Media change requested
#define ATA_REG_ERR_ABRT	0x04	// Aborted command
#define ATA_REG_ERR_NTK0	0x02	// Track 0 not found
#define ATA_REG_ERR_NAM		0x01	// Address mark not found

//---------------------------------------------------------------------------
//	Parameters for Status and Alternate Status registers
//---------------------------------------------------------------------------

#define ATA_REG_STS_BSY		0x80	// Busy
#define ATA_REG_STS_DRDY	0x40	// Device ready
#define ATA_REG_STS_DF		0x20	// Device fault
#define ATA_REG_STS_DSC		0x10	// Device seek complete
#define ATA_REG_STS_DRQ		0x08	// Data request
#define ATA_REG_STS_CORR	0x04	// Corrected data
#define ATA_REG_STS_IDX		0x02	// Index
#define ATA_REG_STS_ERR		0x01	// Error

//---------------------------------------------------------------------------
//	ATA Protocols
//---------------------------------------------------------------------------

typedef struct ATA_DeviceInfo {
    UShort    generalConfig;      //0: General configuration
    UShort    obsolete1;          //1:
    UShort    spConf;             //2: Specific configuration
    UShort    obsolete2;          //3:
    UShort    retired1[2];        //4-5:
    UShort    obsolete3;          //6:
    UShort    reserved1[2];       //7-8: Reserved for CompactFlash
    UShort    retired;            //9:
    UShort    serial[10];         //10-19: Serial Number
    UShort    retired2[2];        //20-21:
    UShort    obsolete4;          //22:
    UShort    revision[4];        //23-26: Firmware revision
    UShort    model[20];          //27-46: Model number
    UShort    maxXferSectors;     //47: Max num sectors transferred
    UShort    reserved2;          //48:
    UShort    cap[2];             //49-50: Capabilities
    UShort    obsolete5[2];       //51-52:
    UShort    validation;         //53:
    UShort    obsolete6[5];       //54-58:
    UShort    curxfer;            //59:
    UShort    totalSectors[2];    //60-61: Total num of user sectors
    UShort    obsolete7;          //62:
    UShort    dmaMode;            //63:
    UShort    pioMode;            //64:
    UShort    minDMA;             //65: Min multiword DMA xfer cycle time
    UShort    defaultDMA;         //66: Default multiword DMA xfer cycle time
    UShort    minPIO;             //67: Min PIO xfer cycle time w/o flow ctrl
    UShort    minPIOFC;           //68:
    UShort    reserved3[6];       //69-74:
    UShort    queueDepth;         //75: Queue depth
    UShort    reserved4[4];       //76-79:
    UShort    majorVersion;       //80: Major version number
    UShort    minorVersion;       //81: Minor version number
    UShort    command[6];         //82-87: Command set/feature supported
    UShort    mode;               //88: Ultra DMA mode
    UShort    eraseTime;          //89: Time for secutiry erase completion
    UShort    xEraseTime;         //90: Time for enhanced security erase
    UShort    apm;                //91: Current APM value
    UShort    passwordRev;        //92: Master password revision code
    UShort    reset;              //93: Hardware reset result
    UShort    acoustic;           //94: Acoustic management
    UShort    minStream;          //95: Stream minimum request size
    UShort    xferTimeDMA;        //96: Streaming transfer time (DMA)
    UShort    latency;            //97: Streaming access latency (DMA & PIO)
    UShort    performance[2];     //98-99: Streaming performance granularity
    UShort    maxLBA[4];          //100-103: Max user LBA
    UShort    xferTimePIO;        //104: Streaming transfer time (PIO)
    UShort    reserved5;          //105:
    UShort    sectorSize;         //106: Physical/logical sector size
    UShort    delay;              //107:
    UShort    oui[2];
    UShort    uid[2];
    UShort    reserved6[5];       //112-116:
    UShort    sectorWords[2];     //117-118: Words per logical sector
    UShort    reserved7[8];       //119-126:
    UShort    mediaStatus;        //127:
    UShort    security;           //128:
    UShort    vendor[31];         //129-159:
    UShort    cfa;                //160: CFA power mode
    UShort    reserved8[15];      //161-175: Reserved for CompaceFlash
    UShort    curSN[30];          //176-205:
    UShort    reserved[49];       //206-254:
    UShort    checksum;           //255: Checksum & signature
} ATA_DeviceInfo_t;

#endif // ARC_DEVICE_ATA_H
