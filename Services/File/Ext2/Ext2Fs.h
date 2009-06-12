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
/// @brief  Ext2 file system
/// @file   Services/File/Ext2/Ext2Fs.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2Fs.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_FILE_EXT2_FS_H
#define ARC_FILE_EXT2_FS_H

class Ext2Partition;
class Ext2File;
class Ext2Inode;
class Ext2Dir;

class Ext2Fs
{
private:
    ///
    /// Maximum length of file name
    ///
    static const UInt   MAX_NAME_LEN = 255;

    ///
    /// Offset of the first block
    ///
    static const UInt   BLOCK_OFFSET = 1;

    ///
    /// Default block size of the file system
    ///
    static const UInt   BLOCK_SIZE = 512;

    ///
    /// The partition where this file system is organized
    ///
    Ext2Partition*      _partition;

    ///
    /// Pointer to the root file of the file system
    ///
    Ext2File*           _root;

    ///
    /// Searches for the specified file in the specified directory.
    ///
    /// @param inode    the current directory
    /// @param query    the file name
    /// @param len      the length of the file name string
    /// @param dir      search only directories if true
    /// @param ino      the inode number that points to the file
    ///
    stat_t SearchFile(Ext2File* inode, const char *query, size_t len,
                      Bool dir, Int* ino);

    ///
    /// Alocates a new directory entry.
    ///
    /// @param head     the head of a list of directory entries
    /// @param size     the length of the file name
    /// @return         an object if there's a free entry, 0 otherwise
    ///
    Ext2Dir* AllocateDirEntryHelper(Ext2Dir *head, size_t size);

    ///
    /// Allocates a new directory entry in the current directory.
    ///
    ///
    /// @param head     the current directory
    /// @param size     the length of the file name
    /// @return         an object if succeeds, 0 otherwise
    ///
    Ext2Dir* AllocateDirEntry(Ext2File* cd, size_t name_len);

    ///
    /// Removes the directory entry
    ///
    void ReleaseDirEntry(UInt ino);

    ///
    /// Removes the characters at the top and/or the end of the string.
    ///
    /// @param str      the string
    /// @param c        the charater to be removed
    ///
    char* Strip(char *str, char c);

    UInt GetFileNameIndex(const char *path);

    char* SubString(const char* str, Int begin, Int end);

    Ext2Fs() {}

public:
    Ext2Fs(Ext2Partition* p);

    ~Ext2Fs();

    ///
    /// Obtains the root file of this partition.
    ///
    const Ext2File* Root();

    ///
    /// Creates a file in the specified directory.
    ///
    /// @param dir      the directory in which the file is created
    /// @param name     the name of the file
    /// @param type     the file type (regular file, directory, etc.)
    /// @return the file object of the file if succeeds, 0 otherwise
    ///
    Ext2File* Create(Ext2File* dir, const char* name, UByte type);

    ///
    /// Removes the file.
    ///
    void Remove(Ext2File* file);

    ///
    /// Changes the current directory to the specified path.  Constructs a new
    /// Ext2File object.
    ///
    /// @param dir      the current directory
    /// @param path     the destination
    ///
    Ext2File* ChangeDirectory(Ext2File* dir, const char* path);

    ///
    /// Opens the specified file in the specified directory.
    ///
    /// @param dir      the directory where the file is supposed to be placed
    /// @param name     the name of the file
    /// @param type     the type of the file (regular, directory, etc.)
    ///
    Ext2File* Open(Ext2File* dir, const char* name, UInt mode);

    ///
    /// Opens a file on the specified path.
    ///
    /// @param path     the path to the file
    /// @param type     the type of the file (regular, directory, etc.)
    ///
    Ext2File* Open(const char* path, UInt mode);

    ///
    /// Checks if the file exists.
    ///
    /// @param dir      the directory where the file is supposed to be placed
    /// @param name     the name of the file to be checked
    /// @param mode     the mode of the file
    ///
    Bool Access(Ext2File* dir, const char* name, UInt mode);

    ///
    /// Checks if the file exists on the specified path.
    ///
    /// @param path     the path to the file
    /// @param mode     the mode of the file
    ///
    Bool Access(const char* path, UInt mode);
};

#endif  // ARC_FILE_EXT2_FS_H

