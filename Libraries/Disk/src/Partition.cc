/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @file   Services/File/Partition.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: Partition.cc 390 2008-08-30 07:15:54Z hro $

#include <Debug.h>
#include <Disk.h>
#include <Types.h>
#include <sys/Config.h>


stat_t
Partition::Open(UInt number)
{
    MBR             mbr;
    unsigned char*  ptr;

    if (number >= 4) {
        return ERR_INVALID_ARGUMENTS;
    }

    _disk->Read(&mbr, 1, 0);

    ptr = mbr.partitionTable[number].start;
    _sectorNumber = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);

    ptr = mbr.partitionTable[number].size;
    _sectorCount = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);

    _type = (PartitionType)mbr.partitionTable[number].type;
    _id = number;

    return ERR_NONE;
}

stat_t
Partition::Read(void *buf, UInt block, size_t block_count)
{
    UInt        scount;
    UInt        soff;
    stat_t    err;
    
    ENTER;

    scount = block_count * _blockSize / Disk::SECTOR_SIZE;
    soff = _sectorNumber + block * (_blockSize / Disk::SECTOR_SIZE);
    err = _disk->Read(buf, soff, scount);

    EXIT;
    return err;
}

stat_t
Partition::Write(const void *buf, UInt block, size_t block_count)
{
    UInt        scount;
    UInt        soff;
    stat_t    err;

    ENTER;

    scount = block_count * _blockSize / Disk::SECTOR_SIZE;
    soff = _sectorNumber + _offset + block * _blockSize / Disk::SECTOR_SIZE;
    err = _disk->Write(buf, soff, scount);

    EXIT;
    return err;
}

