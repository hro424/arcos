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
/// @brief  Data block bitmap management
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Ext2DataBlock.h 369 2008-08-04 08:52:12Z hro $

#ifndef ARC_FILE_EXT2_DATA_BLOCK_H
#define ARC_FILE_EXT2_DATA_BLOCK_H

class Bitmap;
class Ext2SuperBlock;
class Ext2GroupDesc;
class Ext2Partition;

class Ext2DataBlockAllocator
{
private:
    Ext2Partition*  _partition;
    Ext2SuperBlock* _superblock;
    Ext2GroupDesc*  _group_desc;
    Bitmap*         _table;
    UInt            _start;

public:
    ///
    /// Creates a copy of a data block bitmap
    ///
    Ext2DataBlockAllocator(Ext2Partition* p,
                           Ext2SuperBlock* sb,
                           Ext2GroupDesc* gd);

    ///
    /// Deletes the copy.  This method doesn't synchronize the bitmap.
    ///
    virtual ~Ext2DataBlockAllocator();

    ///
    /// Allocates the count of data blocks
    ///
    /// @return the first block number of the chunk
    ///
    UInt Allocate(size_t count);

    ///
    /// Releases the count of data block
    ///
    /// @param block    the first block number of the chunk
    /// @param count    the size of the chunk in blocks
    ///
    void Release(UInt block, size_t count);

    ///
    /// Writes back the allocation state (the data block bitmap) to the disk.
    /// Doesn't write back data blocks.
    ///
    void Sync();
};

#endif // ARC_FILE_EXT2_DATA_BLOCK_H

