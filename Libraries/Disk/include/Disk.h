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
/// @file   Include/arc/Disk.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Disk.h 383 2008-08-28 06:49:02Z hro $

#ifndef ARC_FILE_DISK_H_
#define ARC_FILE_DISK_H_

#include <Session.h>
#include <Types.h>
#include <l4/types.h>

struct PartitionInfo
{
    UByte           active;
    UByte           begin[3];           // Begin CHS
    UByte           type;               // Type of partition
    UByte           end[3];             // End CHS
    UByte           start[4];           // Partition starting sector
    UByte           size[4];            // Size of partition
};

struct MBR
{
    UByte           codeArea[440];
    UByte           serialNumber[4];
    UByte           reserved[2];
    PartitionInfo   partitionTable[4];
    UByte           signature[2];
};

class Partition;

class Disk
{
private:
    static const UInt   SHM_COUNT = 0;
    L4_ThreadId_t       _tid;
    UInt                _address;
    UInt                _iface;
    UInt                _dev;
    Session             *_session;

    void DumpMBR(MBR* mbr) const;
    void DumpPartition(MBR* mbr, int n) const;

public:
    static const UInt SECTOR_SIZE = 512;

    virtual ~Disk();

    stat_t Initialize(const char* name, UInt iface, UInt dev);

    stat_t Open();

    stat_t Close();

    ///
    /// Reads the data at the specified sector to the buffer.
    ///
    /// @param buffer       the buffer where the data is put
    /// @param sector       the sector to be read
    /// @param count        the count of *sectors* to be read
    ///
    stat_t Read(void* buffer, UInt sector, size_t count);

    ///
    /// Writes the data to the specified sector.
    ///
    /// @param buffer       the data to be written to the disk
    /// @param sector       the sector where the data is written
    /// @param count        the count of *sectors* to be written
    ///
    stat_t Write(const void* buffer, UInt sector, size_t count);

    Partition* GetPartition(UInt vol, UInt type);

    void ReleasePartition(Partition *p);

    L4_ThreadId_t Id() { return _tid; }

    void DumpPartitions();
};

class Partition {
public:
    enum PartitionType
    {
        EMPTY =                 0x00,
        FAT12 =                 0x01,
        FAT16_32M =             0x04,
        EXTENDED =              0x05,
        FAT16 =                 0x06,
        HPFS =                  0x07,
        NTFS =                  0x07,
        W95_FAT32 =             0x0B,
        W95_FAT32_LBA =         0x0C,
        W95_FAT16_LBA =         0x0E,
        W95_EXTENDED =          0x0F,
        HIDDEN_FAT12 =          0x11,
        HIDDEN_FAT16_32M =      0x14,
        HIDDEN_FAT16 =          0x16,
        HIDDEN_HPFS =           0x17,
        HIDDEN_NTFS =           0x17,
        HIDDEN_FAT32 =          0x1B,
        HIDDEN_FAT32_LBA =      0x1C,
        HIDDEN_FAT16_LBA =      0x1D,
        PLAN9 =                 0x39,
        PARTITION_MAGIC =       0x3C,
        SFS =                   0x42,
        SYSV =                  0x63,
        MACH =                  0x63,
        HURD =                  0x63,
        MINIX =                 0x81,
        LINUX_SWAP =            0x82,
        LINUX =                 0x83,
        NEXTSTEP =              0xA7,
        DARWIN_UFS =            0xA8,
        NETBSD =                0xA9,
        DARWIN_BOOT =           0xAB,
        DARWIN_HFS =            0xAF,
        BSDI =                  0xB7,
        BSDI_SWAP =             0xB8,
        SOLARIS_BOOT =          0xBE,
        SOLARIS =               0xBF,
        BEOS =                  0xEB,
    };

    static const int TABLE_LENGTH = 4;

private:
    Disk*           _disk;
    UInt            _id;

    ///
    /// Type of the volume
    ///
    PartitionType   _type;

    ///
    /// The starting sector of the volume
    ///
    UInt            _sectorNumber;

    ///
    /// The size of the volume
    ///
    UInt            _sectorCount;

    UInt            _offset;

    ///
    /// The size of a block in the volume
    ///
    UInt            _blockSize;

public:
    UInt Offset() const { return _offset; }

    void SetBlockSize(size_t size) {
        _blockSize = size;
    }

    UInt BlockSize() const { return _blockSize; }

    stat_t Initialize(Disk* disk, UInt blockSize);

    stat_t Open(UInt number);

    stat_t Close();

    ///
    /// Reads the block of data into the buffer.
    ///
    /// @param vol	the volume to be read
    /// @param buf	the buffer storing data. Must be enough big to store the
    /// 		data.
    /// @param boff	offset in blocks from the top of the volume
    /// @param bcnt	the number of blocks to be read
    ///
    stat_t Read(void *buf, UInt block, size_t count);

    stat_t Write(const void *buf, UInt block, size_t count);

    stat_t ReadBlock(void *buf, UInt block);

    stat_t WriteBlock(const void *buf, UInt block);

    UInt Block2Sector(UInt num) const {
        return num * _blockSize / Disk::SECTOR_SIZE;
    }

    PartitionType Type() const { return _type; }

    friend class Disk;
};

#endif  // ARC_FILE_DISK_H

