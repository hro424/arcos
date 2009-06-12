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
/// @brief  Ext2 partition representation
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: Ext2Partition.h 369 2008-08-04 08:52:12Z hro $

#ifndef ARC_FILE_EXT2_PARTITION_H
#define ARC_FILE_EXT2_PARTITION_H

class Partition;
class Ext2SuperBlock;
class Ext2GroupDesc;
class Ext2DataBlockAllocator;
class Ext2Inode;
class Ext2InodeAllocator;
class Ext2InodeTable;

class Ext2Partition
{
private:
    static const UInt   BLOCK_OFFSET = 1;
    static const UInt   BLOCK_SIZE = 512;

    ///
    /// The disk partition
    ///
    Partition*                  _partition;

    ///
    /// The superblock of this partition
    ///
    Ext2SuperBlock*             _superblock;

    ///
    /// The block group descriptors of this partition
    ///
    Ext2GroupDesc*              _group_desc;

    ///
    /// The number of block groups
    ///
    UInt                        _groups;

    ///
    /// The data block bitmap
    ///
    Ext2DataBlockAllocator**    _data_block_allocator;

    ///
    /// The inode bitmap
    ///
    Ext2InodeAllocator**        _inode_allocator;

    ///
    /// The inode table
    ///
    Ext2InodeTable**            _inode_table;

    ///
    /// Probes if the given partition is formated in Ext2.  If so, gets the
    /// superblock.
    ///
    Ext2SuperBlock *Probe(Partition *p);

    ///
    /// Initializes the block group descriptors of this partition.
    ///
    stat_t InitGroupDescriptors();

    ///
    /// Initializes all the data block bitmaps of this partition.
    ///
    stat_t InitDataBlockBitmap();

    ///
    /// Initializes all the inode bitmaps of this partition.
    ///
    stat_t InitInodeBitmap();

    ///
    /// Initializes all the inode tables of this partition.
    ///
    stat_t InitInodeTable();

public:
    ///
    /// Initializes the Ext2 partition object.
    ///
    stat_t Initialize(Partition *p);

    ///
    /// Allocates the count of data blocks for the specified file.
    ///
    /// @param ino          the file inode number
    /// @param count        the count of blocks to be allocated
    ///
    UInt AllocateDataBlock(Int ino, size_t count);

    ///
    /// Releases the count of data blocks.
    ///
    void ReleaseDataBlock(Int ino, UInt blkno, size_t count);

    ///
    /// Creates a new file.
    ///
    Int AllocateInode();

    ///
    /// Remove the file.
    ///
    void ReleaseInode(Int ino);

    ///
    /// Obtains the inode specified by the inode number.
    ///
    void Read(Int ino, Ext2Inode* inode);

    ///
    /// Updates the inode.
    ///
    void Write(Int ino, const Ext2Inode* inode);

    ///
    /// Obtains the block size of this partition.
    ///
    size_t BlockSizeLog2();

    ///
    /// Obtains the block size of this partition.
    ///
    size_t BlockSize();

    ///
    /// Read the contents of the specified block into the buffer.
    /// NOTE: buffer has to be block size.
    ///
    stat_t ReadBlock(void* buffer, UInt block);

    ///
    /// Write the given data to the specified block.
    /// NOTE: buffer has to be block size.
    ///
    stat_t WriteBlock(const void* buffer, UInt block);

    ///
    /// Synchronizes the data with the superblock.
    ///
    void SyncSuperBlock();

    ///
    /// Synchronizes the internal dta with the group descriptors.
    ///
    void SyncGroupDescriptors();

    ///
    /// Allocates a cache block
    ///
    void* AllocateCache();

    ///
    /// Releases the cache
    ///
    void ReleaseCache(void* ptr);

    ///
    /// Allocates cache and read data to it.
    ///
    stat_t ReadBlockAndAllocate(void** buf, UInt block);

    ///
    /// Writes the cache back to the disk and releases the cache.
    ///
    stat_t WriteBlockAndRelease(const void* ptr, UInt block);

    void DumpGroups();
};

#endif // ARC_FILE_EXT2_PARTITION_H

