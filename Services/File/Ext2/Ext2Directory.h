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
/// @file   Services/File/Ext2/Ext2Directory.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2Directory.h 369 2008-08-04 08:52:12Z hro $


#ifndef ARC_FILE_EXT2_DIRECTORY_H
#define ARC_FILE_EXT2_DIRECTORY_H

#include <arc/server.h>


struct Ext2Dir
{
    ///
    /// The length of the header field of a record.
    /// sizeof(inode) + sizeof(rec_len) + sizeof(name_len) + sizeof(file_type)
    ///
    static const UByte  HEADER_LENGTH = 8;

    ///
    /// The minimum record length.
    ///
    static const UShort MIN_RECORD_LENGTH = HEADER_LENGTH + 4;

    ///
    /// The maximum length of a file name
    ///
    static const UShort MAX_NAME_LENGTH = 255;

    enum FileType {
        UNKNOWN = 0,
        REGULAR = 1,
        DIRECTORY = 2,
        CHARDEV = 3,
        BLKDEV = 4,
        PIPE = 5,
        SOCKET = 6,
        SYMLINK = 7,
    };

    ///
    /// The inode number
    ///
    UInt    inode;

    ///
    /// The length of this record
    ///
    UShort  recordLength;

    ///
    /// The length of the file name
    ///
    UByte   nameLength;

    ///
    /// The type of the record
    ///
    UByte   fileType;

    ///
    /// The file name
    ///
    char    name[];

    ///
    /// Gets the record length out of the name length.
    ///
    static UShort CalcRecordLength(size_t size) {
        return HEADER_LENGTH + ROUND_UP(size, 4);
    }

    void Print() const {
        DOUT("ino:%lu rec_len:%u name_len:%u type:%u name:%s\n",
             inode, recordLength, nameLength, fileType, name);
    }
};

#endif // ARC_FILE_EXT2_DIRECTORY_H

