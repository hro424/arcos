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
/// @file   Services/File/Ext2/Ext2File.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2File.h 424 2008-09-30 08:47:19Z hro $

#ifndef ARC_SERVICES_FILE_EXT2_FILE_H
#define ARC_SERVICES_FILE_EXT2_FILE_H

class Ext2Inode;
class Ext2Partition;
class Ext2FsServer;

class Ext2File
{
public:
    enum Mode {
        // Creates the file
        CREATE =        0x001,
        // Write to the file
        WRITE =         0x002,
        // Read the file
        READ =          0x004,
        // Trancate the file length to 0
        TRUNCATE =      0x010,
        // Write to the file.  Expand it if data exceeds the file.
        APPEND =        0x020,
        // Check if the file is already exists
        EXISTENCE =     0x040,

        DIRECTORY =     0x080,
    };

private:
    ///
    /// The partition this file is stored
    ///
    Ext2Partition*  _partition;

    ///
    /// The inode of this file
    ///
    Ext2Inode*      _inode;

    ///
    /// The inode number of this file
    ///
    Int             _ino;

    ///
    /// File operation mode
    ///
    UInt             _mode;

    stat_t MapBlock(UInt file_block, UInt* logical_block);

    stat_t GetDataBlock(UInt offset, UInt* logical_block);

    stat_t WriteAppend(const void* const buf, size_t count, UInt offset,
                         size_t* wsize);

    stat_t WriteNonAppend(const void* const buf, size_t count, UInt offset,
                            size_t* wsize);

public:
    Ext2File();

    Ext2File(Ext2Partition* p, Ext2Inode& inode, Int ino, UInt mode);

    ~Ext2File();

    void Close();

    void Copy(const Ext2File* f);

    ///
    /// Read data in the file.
    ///
    /// @param buf      the buffer to which the data is copied
    /// @param count    the size of the buffer
    /// @param offset   the offset in the file
    /// @param rsize    the size of data read
    ///
    stat_t Read(void *buf, size_t count, UInt offset, size_t *rsize);

    stat_t Write(const void* const buf, size_t count, UInt offset,
                 size_t *wsize);

    stat_t Seek(size_t count, UInt mode);

    stat_t Flush();

    const Ext2Inode* Inode();

    Int Ino();

    size_t Size();

    friend class Ext2FsServer;
};

#endif // ARC_SERVICES_FILE_EXT2_FILE_H

