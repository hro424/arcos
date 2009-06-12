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
/// @file   Services/File/Ext2/Ext2File.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2File.cc 424 2008-09-30 08:47:19Z hro $

#include <arc/server.h>
#include "Ext2File.h"
#include "Inode.h"
#include "Ext2Partition.h"


// Translate a file block number to a logical block number
stat_t
Ext2File::MapBlock(UInt blkno, UInt* blkptr)
{
    static char buf[PAGE_SIZE];     //FIXME: should be block size
    size_t      log_num_blocks;
    UInt        *block;
    stat_t    err;

    // from 0 to 11
    if (blkno < Ext2Inode::NDIR_BLOCK) {
        *blkptr = _inode->data[blkno];
        return ERR_NONE;
    }

    // Adjust the block number to omit the block No.0 to 11
    blkno -= Ext2Inode::NDIR_BLOCK;

    // b / 4 == b_in_log - 2
    log_num_blocks = _partition->BlockSizeLog2() - 2;

    // from 12 to b/4 + 11, where b is the filesystem's block size.
    // (we can ignore +11 because it has already been adjusted above.)
    if (blkno < (1U << log_num_blocks)) {
        err = _partition->ReadBlock(buf,
                                    _inode->data[Ext2Inode::DINDIR_BLOCK]);
        if (err != ERR_NONE) {
            return err;
        }

        *blkptr = ((UInt *)buf)[blkno];
        return ERR_NONE;
    }

    // from b/4 + 12 to (b/4)^2 + b/4 + 11
    blkno -= 1U << log_num_blocks;
    if (blkno < (1U << (log_num_blocks * 2))) {
        err = _partition->ReadBlock(buf,
                                    _inode->data[Ext2Inode::TINDIR_BLOCK]);
        if (err != ERR_NONE) {
            return err;
        }

        block = (UInt *)buf;
        err = _partition->ReadBlock(buf, block[blkno >> log_num_blocks]);
        if (err != ERR_NONE) {
            return err;
        }

        block = (UInt *)buf;
        *blkptr = block[blkno & ((1 << log_num_blocks) -1)];
        return ERR_NONE;
    }

    // from (b/4)^2 + b/4 + 12 to (b/4)^3 + (b/4)^2 + b/4 + 11
    blkno -= 1U << (log_num_blocks * 2);
    if (blkno < (1U << (log_num_blocks * 3))) {
        err = _partition->ReadBlock(buf,
                                    _inode->data[Ext2Inode::QINDIR_BLOCK]);
        if (err != ERR_NONE) {
            return err;
        }

        block = (UInt *)buf;
        _partition->ReadBlock(buf, block[blkno >> (log_num_blocks * 2)]);
        if (err != ERR_NONE) {
            return err;
        }

        block = (UInt *)buf;
        *blkptr = block[blkno & ((1 << log_num_blocks) -1)];
        return ERR_NONE;
    }

    return ERR_NOT_FOUND;
}

stat_t
Ext2File::GetDataBlock(UInt offset, UInt* block)
{
    UInt        file_block;

    if (offset >= _inode->size) {
        return ERR_INVALID_ARGUMENTS;
    }

    file_block = offset >> _partition->BlockSizeLog2();
    return MapBlock(file_block, block);
}

Ext2File::Ext2File() : _partition(0), _ino(0), _mode(0)
{
}

Ext2File::Ext2File(Ext2Partition* p, Ext2Inode& inode, Int ino, UInt mode)
    : _partition(p), _ino(ino), _mode(mode)
{
    _inode = new Ext2Inode(inode);
}

Ext2File::~Ext2File()
{
    Close();
}

const Ext2Inode*
Ext2File::Inode()
{
    return _inode;
}

Int
Ext2File::Ino()
{
    return _ino;
}

void
Ext2File::Close()
{
    if (_inode == 0) {
        return;
    }

    Flush();
    delete _inode;
}

void
Ext2File::Copy(const Ext2File* file)
{
    _partition = file->_partition;
    _ino = file->_ino;
    _mode = file->_mode;
}

stat_t
Ext2File::Read(void* buf, size_t count, UInt offset, size_t* rsize)
{
    // TODO: use no buffer
    static char read_buf[PAGE_SIZE];
    ENTER;

    if (_inode == 0) {
        // The file is not opened.
        return ERR_NOT_FOUND;
    }

    if (offset >= _inode->size) {
        // The offset exceeds the end of the file.
        if (rsize != 0) {
            *rsize = 0;
        }
        return ERR_NONE;
    }

    if ((_inode->size - offset) < count) {
        count = _inode->size - offset;
    }

    UInt cursor = offset;
    while (cursor < (offset + count)) {
        UInt    start, remain, block, copy_size;
        stat_t  err;

        if ((err = GetDataBlock(cursor, &block)) != ERR_NONE) {
            DOUT("%s\n", stat2msg[err]);
            return ERR_NOT_FOUND;
        }

        if ((err = _partition->ReadBlock(read_buf, block)) != ERR_NONE) {
            DOUT("%s\n", stat2msg[err]);
            break;
        }

        // Calculate the size of data to be copied
        start = cursor & (_partition->BlockSize() - 1);
        remain = count - (cursor - offset);
        if (remain < _partition->BlockSize()) {
            copy_size = remain;
        }
        else {
            copy_size = _partition->BlockSize() - start;
        }

        // Copy to the client's buffer
        memcpy(static_cast<char*>(buf) + cursor - offset,
               read_buf + start, copy_size);

        cursor += copy_size;
    }

    if (rsize != 0) {
        *rsize = cursor - offset;
    }
    EXIT;
    return ERR_NONE;
}

stat_t
Ext2File::WriteAppend(const void* const buf, size_t count, UInt offset,
                      size_t* wsize)
{
    if ((offset + count) >= _inode->size) {
        // Expand the file. Assume the block size is power of 2.
        size_t blocks = offset + count -
                        ROUND_UP(_inode->size, _partition->BlockSize());
        _partition->AllocateDataBlock(_ino, blocks);
    }

    return WriteNonAppend(buf, count, offset, wsize);
}

stat_t
Ext2File::WriteNonAppend(const void* const buf, size_t count, UInt offset,
                         size_t* wsize)
{
    static char write_buf[PAGE_SIZE];
    stat_t    err = ERR_NONE;

    if (offset >= _inode->size) {
        // The offset exceeds the end of the file.
        if (wsize != 0) {
            *wsize = 0;
        }
        return ERR_NONE;
    }

    UInt cursor = offset;
    while (cursor < (offset + count)) {
        UInt start, remain, block, copy_size;

        // Fetch the data block
        if (GetDataBlock(cursor, &block) != ERR_NONE) {
            return ERR_NOT_FOUND;
        }

        if ((err = _partition->ReadBlock(write_buf, block)) != ERR_NONE) {
            break;
        }

        start = cursor & (_partition->BlockSize() - 1);
        remain = count - (cursor - offset);
        if (remain < _partition->BlockSize()) {
            copy_size = remain;
        }
        else {
            copy_size = _partition->BlockSize() - start;
        }

        // Update the data block
        memcpy(write_buf + start,
               static_cast<const char* const>(buf) + cursor - offset,
               copy_size);

        // Write back the data block
        if ((err = _partition->WriteBlock(write_buf, block)) != ERR_NONE) {
            break;
        }

        cursor += copy_size;
    }

    if (wsize != 0) {
        *wsize = cursor - offset;
    }
    return err;
}


stat_t
Ext2File::Write(const void* const buf, size_t count, UInt offset,
                size_t* wsize)
{
    if (_inode == 0) {
        // File is not opened.
        return ERR_NOT_FOUND;
    }

    if ((_mode & APPEND) > 0) {
        return WriteAppend(buf, count, offset, wsize);
    }
    else if ((_mode & WRITE) > 0) {
        return WriteNonAppend(buf, count, offset, wsize);
    }
    else {
        return ERR_INVALID_RIGHTS;
    }
}

stat_t
Ext2File::Seek(size_t count, UInt mode)
{
    if (_inode == 0) {
        return ERR_NOT_FOUND;
    }

    return ERR_NONE;
}

stat_t
Ext2File::Flush()
{
    if (_inode == 0) {
        return ERR_NOT_FOUND;
    }

    return ERR_NONE;
}

size_t
Ext2File::Size()
{
    return _inode->size;
}

