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
/// @file   Services/File/Ext2/Inode.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Inode.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_FILE_EXT2_INODE_H
#define ARC_FILE_EXT2_INODE_H

#include <Types.h>
#include <String.h>

struct Ext2Inode
{
    static const UInt ROOT = 2;
    static const UInt NBLOCKS = 15;
    static const UInt NDIR_BLOCK = 12;
    static const UInt DINDIR_BLOCK = NDIR_BLOCK;
    static const UInt TINDIR_BLOCK = DINDIR_BLOCK + 1;
    static const UInt QINDIR_BLOCK = TINDIR_BLOCK + 1;

    enum FileMode {
        IAEXEC =    0x0040,
        IAWRITE =   0x0080,
        IAREAD =    0x0100,
        IASTICK =   0x0200,
        IASGID =    0x0400,
        IASUID =    0x0800,
        IFMASK =    0xF000,
        IFIFO =     0x1000,
        IFCHR =     0x2000,
        IFDIR =     0x4000,
        IFBLK =     0x6000,
        IFREG =     0x8000,
        IFLNK =     0xA000,
        IFSOCK =    0xC000,
    };

    UShort      mode;
    UShort      uid;
    UInt        size;
    UInt        atime;
    UInt        ctime;
    UInt        mtime;
    UInt        dtime;
    UShort      gid;
    UShort      links_count;
    UInt        blocks;
    UInt        flags;
    UInt        osinfo1;
    UInt        data[NBLOCKS];
    UInt        gen;
    UInt        facl;
    UInt        dacl;
    UInt        faddr;
    UInt        osinfo2[3];

    Ext2Inode();
    Ext2Inode(Ext2Inode& inode);
    //void Print() const;

    Ext2Inode& operator=(const Ext2Inode& right);
};

inline
Ext2Inode::Ext2Inode()
{
}

inline
Ext2Inode::Ext2Inode(Ext2Inode& inode)
{
    memcpy(this, &inode, sizeof(Ext2Inode));
}

inline Ext2Inode&
Ext2Inode::operator=(const Ext2Inode& right)
{
    memcpy(this, &right, sizeof(Ext2Inode));
    return *this;
}


/*
inline void
Ext2Inode::Print() const
{
    ConsoleOut(INFO, "-- Inode --\n");

    ConsoleOut(INFO, "File mode: 0x%X\n", mode);
    ConsoleOut(INFO, "File owner: %u\n", uid);
    ConsoleOut(INFO, "File size: %lu bytes\n", size);
    ConsoleOut(INFO, "File blocks: %lu blocks\n", blocks);

    ConsoleOut(INFO, "File blocks:");
    for (int i = 0; i < 11; i++) {
        ConsoleOut(INFO, "%lu ", data[i]);
    }

    ConsoleOut(INFO, "\n-- Dump Inode End --\n");
}
*/

#endif  // ARC_FILE_EXT2_INODE_H

