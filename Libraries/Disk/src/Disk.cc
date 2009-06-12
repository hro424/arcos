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
/// @file   Library/Disk/Disk.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Disk.cc 390 2008-08-30 07:15:54Z hro $

//#define SYS_DEBUG
//#define SYS_DEBUG_CALL

#include <Debug.h>
#include <Disk.h>
#include <MemoryAllocator.h>
#include <NameService.h>
#include <Ipc.h>
#include <String.h>
#include <Types.h>
#include <sys/Config.h>

#include <l4/message.h>
#include <l4/ipc.h>
#include <l4/types.h>


Disk::~Disk()
{
    delete _session;
}

stat_t
Disk::Initialize(const char *name, UInt iface, UInt dev)
{
    ENTER;
    NameService::Get(name, &_tid);
    _iface = iface;
    _dev = dev;
    _session = new Session(_tid);
    if (_session == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    if (!_session->IsConnected()) {
        return ERR_UNKNOWN;
    }

    EXIT;
    return ERR_NONE;
}

// iface: 0, 1
// dev: 0 - 3
stat_t
Disk::Open()
{
    ENTER;

    //
    // Allocate shared memory for communicating with the disk controller
    //
    _address = _session->GetBaseAddress();
    DOUT("base address: %.8lX\n", _address);

    //
    // Connects to the device
    //
    /*
    L4_Word_t   reg = (_iface << 16) | _dev;
    stat_t      err = _session->Begin(&reg, 1);
    if (err != ERR_NONE) {
        return err;
    }
    */

    EXIT;
    return ERR_NONE;
}

stat_t
Disk::Close()
{
    ENTER;

    //
    // Get rid of the current session
    //
    /*
    stat_t err = _session->End();
    if (err != ERR_NONE) {
        return err;
    }
    */

    EXIT;
    return ERR_NONE;
}

stat_t
Disk::Read(void *buffer, UInt sector, size_t seccnt)
{
    int         sec_per_shm;    // in sectors
    int         cpylen;         // in bytes
    int         bufcnt;         // in sectors
    char*       ptr;
    stat_t      err;

    ENTER;

    DOUT("buf: %p sector: %lu count: %d\n", buffer, sector, seccnt);

    ptr = (char *)buffer;
    bufcnt = (int)seccnt;
    sec_per_shm = _session->Size() / SECTOR_SIZE;

    while (0 < bufcnt) {
        L4_Word_t reg[3];
        reg[0] = (_iface << 16) | _dev;
        reg[1] = sector;
        reg[2] = sec_per_shm;
        err = _session->Get(reg, 3);
        if (err != ERR_NONE) {
            return err;
        }

        if (bufcnt < sec_per_shm) {
            cpylen = bufcnt * SECTOR_SIZE;
        }
        else {
            cpylen = sec_per_shm * SECTOR_SIZE;
        }

        memcpy(ptr, reinterpret_cast<void*>(_session->GetBaseAddress()),
               cpylen);
        ptr += cpylen;
        bufcnt -= sec_per_shm;
        sector += sec_per_shm;
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Disk::Write(const void *buffer, UInt sector, size_t count)
{
    stat_t        err;
    ENTER;

    memcpy(reinterpret_cast<void*>(_address), buffer, count);

    L4_Word_t reg[3];
    reg[0] = (_iface << 16) | _dev;
    reg[1] = sector;
    reg[2] = count;
    err = _session->Put(reg, 3);
    if (err != ERR_NONE) {
        return err;
    }

    EXIT;
    return ERR_NONE;
}

void
Disk::DumpMBR(MBR *mbr) const
{
    DOUT("-- Dump MBR Here --\n");
    const char *ptr = (const char *)mbr;
    for (int i = 0; i < 512; i += 16) {
        Debug.Print("%.8X ", i);
        for (int j = i; j < i + 16; j++) {
            Debug.Print("%.2X ", ptr[j] & 0xFF);
        }
        System.Print("\n");
    }
    DOUT("-- Dump MBR End --\n");
}

void
Disk::DumpPartition(MBR *mbr, int n) const
{
    PartitionInfo   *info;
    UInt            start;
    UInt            size;

    info = &mbr->partitionTable[n];
    Debug.Print("-- Partition %d (%s)--\n",
                n, info->active != 0 ? "active" : "inactive");
    start = info->start[0] | (info->start[1] << 8) | (info->start[2] << 16) |
        (info->start[3] << 24);
    size = info->size[0] | (info->size[1] << 8) | (info->size[2] << 16) |
        (info->size[3] << 24);
    Debug.Print("type: %X, start: %lu, size: %lu sectors\n",
                info->type, start, size);
}

void
Disk::DumpPartitions()
{
    MBR mbr;

    Read(&mbr, 0, 1);

    DumpMBR(&mbr);

    for (int i = 0; i < 4; i++) {
        DumpPartition(&mbr, i);
    }
}

Partition*
Disk::GetPartition(UInt num, UInt type)
{
    MBR             mbr;
    unsigned char   *ptr;
    Partition       *partition = 0;

    ENTER;

    if (num < 0 || 3 < num) {
        return 0;
    }

    if (Read(&mbr, 0, 1) != ERR_NONE) {
        return 0;
    }

    if (mbr.partitionTable[num].type == type) {
        // Create a partition object
        partition = (Partition*)malloc(sizeof(Partition));

        ptr = mbr.partitionTable[num].start;
        partition->_sectorNumber = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) |
                                (ptr[3] << 24);
        ptr = mbr.partitionTable[num].size;
        partition->_sectorCount = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) |
                                (ptr[3] << 24);
        partition->_type =
                (Partition::PartitionType)mbr.partitionTable[num].type;
        partition->_disk = this;
        partition->_id = num;
        partition->_blockSize = 0U;
    }

    EXIT;

    return partition;
}

void
Disk::ReleasePartition(Partition *p)
{
    mfree(p);
}

