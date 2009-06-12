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
/// @file   Services/File/Ext2/Ext2SuperBlock.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2SuperBlock.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_FILE_EXT2_SUPER_BLOCK_H
#define ARC_FILE_EXT2_SUPER_BLOCK_H

class Ext2SuperBlock {
public:
    UInt	inodes;			/*0: Total number of inodes */
    UInt	blocks;			/*4: FS size in blocks */
    UInt	reservedBlocks;		/*8: Num of reserved blocks */
    UInt	freeBlocks;		/*12: Free blocks counter */
    UInt	freeInodes;		/*16: Free inodes counter */
    UInt	firstDataBlock;         /*20: */
    UInt	logBlockSize;		/*24: Block size */
    UInt        logFragSize;		/*28: Fragment size */
    UInt	blocksPerGroup;		/*32: Num of blocks/group */
    UInt	fragsPerGroup;		/*36: Num of fragments/group */
    UInt	inodesPerGroup;		/*40: Num of inodes/group */
    UInt	mtime;			/*44: Time of last mount */
    UInt	wtime;			/*48: Time of last write */
    UShort	mntCount;		/*52: Mount ops counter */
    UShort	maxMntCount;		/*54: Num of mount ops before check */
    UShort	magic;			/*56: Magic signature */
    UShort	state;			/*58: Status flag */
    UShort	errors;			/*60: Behavior when detecting errors */
    UShort	minorRevLevel;		/*62: Minor revision level */
    UInt	lastCheck;		/*64: Time of last check */
    UInt	checkinterval;		/*68: Time between checks */
    UInt	creatorOs;		/*72: OS where FS created */
    UInt	revLevel;		/*76: Revision level */
    UShort	defResuid;		/*80: Default UID for reserved blks */
    UShort	defResgid;		/*82: Default GID for reserved blks */
    UInt	firstIno;		/*84: Num of first nonreserved inode */
    UShort	inodeSize;		/*88: Size of on-disk inode structure */
    UShort	blockGroupNr;		/*90: Block group number */
    UInt	featureCompat;		/*92: Compatible features bitmap */
    UInt	featureIncompat;	/*96: Imcompatible */
    UInt	featureRoCompat;	/*100: Read-only compatible */
    UByte	uuid[16];		/*104: 128-bit FS ID */
    UByte	volumeName[16];		/*120: Volume name */
    UByte	lastMounted[64];	/*136: Path of last mnt pt */
    UInt	algorithmUsageBitmap;	/*200: For compression */
    UByte	preallocBlocks;		/*204: Num of blks */
    UByte	preallocDirBlocks;	/*205: Num of blks for dirs */
    UShort	reserved1;		/*206: */
    UInt	reserved2[204];		/*208: */

    static const UInt   MAGIC = 0xEF53;
    static const UInt   OFFSET = 1;             // Offset in a block group
    static const UInt   BLOCK_SIZE_BASE = 10;   // The base order of block size
    static const UInt   INODE_SIZE = 128;

    static UInt Log2INT8(UInt size) {
        return (1 << ((size) + BLOCK_SIZE_BASE));
    }

    UInt BlockSizeLog2() {
        return (logBlockSize + BLOCK_SIZE_BASE);
    }

    UInt BlockSize() {
        return (1 << ((logBlockSize) + BLOCK_SIZE_BASE));
    }

    UInt BlockMask() {
        return BlockSize() - 1;
    }

    UInt NodesPerBlock() {
        return BlockSize() / INODE_SIZE;
    }

    UInt Groups() {
        return (blocks + 8 * BlockSize() - 1) / (8 * BlockSize());
    }

    void Sync() {}
};                                      /* Total: 1024 bytes */

/* Ext2 Filesystem Group Descriptor */
class Ext2GroupDesc {
public:
    static const UInt   OFFSET = 1;     // Offset in a block group

    UInt	blockBitmap;	/*0: Block number of block bitmap */
    UInt	inodeBitmap;	/*4: Block number of inode bitmap */
    UInt	inodeTable;	/*8: Block number of 1st inode tbl */
    UShort	freeBlocks;	/*12: Number of free blocks */
    UShort	freeInodes;	/*14: Number of free inodes */
    UShort	usedDirs;	/*16: Number of directories */
    UShort	pad;	        /*18: */
    UInt	reserved2[3];	/*20: */
                                /* Total: 32 bytes */

    void Sync() {}
};

#endif // ARC_FILE_EXT2_SUPER_BLOCK_H

