/*
 *
 *  Copyright (C) 2008, Waseda University.
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
/// @brief  Managing data blocks
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Ext2DataBlock.cc 390 2008-08-30 07:15:54Z hro $

#include <arc/server.h>
#include <Bitmap.h>
#include "Ext2DataBlock.h"
#include "Ext2Partition.h"
#include "Ext2SuperBlock.h"

Ext2DataBlockAllocator::Ext2DataBlockAllocator(Ext2Partition* p,
                                               Ext2SuperBlock* sb,
                                               Ext2GroupDesc* gd)
    : _partition(p), _superblock(sb), _group_desc(gd)
{
    stat_t  err;

    // Create a new bitmap
    _table = new Bitmap(_partition->BlockSize() * Bitmap::BITS_PER_BYTE);
    if (_table == 0) {
        FATAL("Ext2: Failed to allocate data block bitmap");
    }

    // Load the current state from the disk to the bitmap.
    // The length of the data block bitmap is always one block.
    err = _partition->ReadBlock(_table->GetMap(), _group_desc->blockBitmap);
    if (err != ERR_NONE) {
        delete _table;
        FATAL("Ext2: Failed to read data block bitmap");
    }

    _start = _superblock->inodesPerGroup / _superblock->NodesPerBlock()
        + _group_desc->inodeTable;
}

Ext2DataBlockAllocator::~Ext2DataBlockAllocator()
{
    delete _table;
}

UInt
Ext2DataBlockAllocator::Allocate(size_t count)
{
    size_t i = 0;
retry:
    for (; i < _table->Length(); i++) {
        if (!_table->Test(i)) {
            break;
        }
    }
    
    for (size_t j = i; j < i + count; j++) {
        if (_table->Test(j)) {
            goto retry;
        }
    }

    for (size_t j = i; j < i + count; j++) {
        _table->Set(j);
    }

    // LOCK
    _superblock->freeBlocks -= count;
    _group_desc->freeBlocks -= count;
    // UNLOCK

    _start += i;
    return _start;
}

void
Ext2DataBlockAllocator::Release(UInt block_no, size_t count)
{
    Int start = block_no - _start;
    for (UInt i = start; i < start + count; i++) {
        _table->Reset(i);
    }
    //TODO: May become inconsistent if the block address is out of range.
    //      Reset() doesn't report any error.
    // LOCK
    _superblock->freeBlocks += count;
    _group_desc->freeBlocks += count;
    // UNLOCK
}

void
Ext2DataBlockAllocator::Sync()
{
    _partition->WriteBlock(_table->GetMap(), _group_desc->blockBitmap);
    _partition->SyncSuperBlock();
    _partition->SyncGroupDescriptors();
}

