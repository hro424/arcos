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
/// @file   Services/File/Ext2/ExtFs.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Ext2Fs.cc 373 2008-08-07 04:43:08Z hro $

#include <arc/server.h>
#include <String.h>
#include "Ext2Directory.h"
#include "Ext2File.h"
#include "Ext2Fs.h"
#include "Ext2Partition.h"
#include "Ext2SuperBlock.h"
#include "Inode.h"

Ext2Fs::Ext2Fs(Ext2Partition *p) : _partition(p)
{
    Ext2Inode inode;
    _partition->Read(Ext2Inode::ROOT, &inode);
    _root = new Ext2File(_partition, inode, Ext2Inode::ROOT, Ext2File::READ);
}

Ext2Fs::~Ext2Fs()
{
    delete _root;
}

const Ext2File*
Ext2Fs::Root()
{
    return _root;
}

stat_t
Ext2Fs::SearchFile(Ext2File *cd, const char *query, size_t qlen, Bool dir,
                   Int *ino)
{
    static char         buf[PAGE_SIZE];
    const Ext2Inode*    inode;
    size_t              rsize;
    stat_t              err;

    ENTER;
    DOUT("query '%s' len %u dir %lu cd %ld\n", query, qlen, dir, cd->Ino());

    inode = cd->Inode();
    if ((inode->mode & Ext2Inode::IFMASK) != Ext2Inode::IFDIR) {
        return ERR_NOT_FOUND;
    }

    do {
        err = cd->Read(buf, PAGE_SIZE, 0, &rsize);
        if (err != ERR_NONE) {
            return err;
        }

        for (UInt i = 0; i < rsize;) {
            Ext2Dir* entry = reinterpret_cast<Ext2Dir*>(buf + i);
            if (entry->recordLength == 0) {
                goto exit;
            }

            DOUT("reclen %u nmlen %u ino %lu type %u str %s\n",
                 entry->recordLength, entry->nameLength,
                 entry->inode, entry->fileType, entry->name);

            if (!dir || entry->fileType == Ext2Dir::DIRECTORY) {
                if (entry->nameLength == qlen &&
                    strncmp(entry->name, query, entry->nameLength) == 0) {
                    *ino = entry->inode;
                    EXIT;
                    return ERR_NONE;
                }
            }
            i += entry->recordLength;
        }
    } while (rsize == PAGE_SIZE);

exit:
    DOUT("not found\n");
    return ERR_NOT_FOUND;
}

#define DIR_ENTRY(base, off)                               \
    reinterpret_cast<Ext2Dir*>(reinterpret_cast<char*>(base) + off)

Ext2Dir*
Ext2Fs::AllocateDirEntryHelper(Ext2Dir* head, size_t name_len)
{
    Ext2Dir*    cur = head;
    Ext2Dir*    prev = 0;
    Ext2Dir*    end = DIR_ENTRY(head, _partition->BlockSize());
    UShort      rec_len = Ext2Dir::CalcRecordLength(name_len);

    // Look for a hole that has enough size to store a new entry
    while (cur < end && cur->recordLength != 0) {
        // The entry is empty? and the size of the entry is enough?
        if (cur->inode == 0 && cur->recordLength >= rec_len) {

            // Create another empty entry if there's still a room
            UShort next_rec_len = cur->recordLength - rec_len;
            if (next_rec_len >= Ext2Dir::MIN_RECORD_LENGTH) {
                Ext2Dir* next = DIR_ENTRY(cur, rec_len); 
                next->recordLength = next_rec_len;
                next->inode = 0;
                next->nameLength = next_rec_len - Ext2Dir::HEADER_LENGTH;
                next->fileType = 0;
            }

            // The value of the length of the previous entry includes
            // the length of its next empty entries.  Adjust it by
            // subtracting the length of the entry just allocated.
            if (prev != 0) {
                prev->recordLength -= rec_len;
            }

            return cur;
        }

        if (cur->inode != 0) {
            // Proceeds to the next entry
            prev = cur;
            cur = DIR_ENTRY(cur, Ext2Dir::CalcRecordLength(cur->nameLength));
        }
        else {
            cur = DIR_ENTRY(cur, cur->recordLength);
        }
    }

    // There's no hole. Allocate a new entry
    if (DIR_ENTRY(cur, rec_len) < end) {
        cur->recordLength = rec_len;
        return cur;
    }

    // Need to allocate a new data block for dir entries
    return 0;
}

Ext2Dir*
Ext2Fs::AllocateDirEntry(Ext2File* cd, size_t name_len)
{
    if (name_len > Ext2Dir::MAX_NAME_LENGTH) {
        return 0;
    }

    /*
    if ((cd->mode & Ext2Inode::IFMASK) != Ext2Inode::IFDIR) {
        return 0;
    }
    */

    //XXX: static buffer
    static char buf[PAGE_SIZE];
    Ext2Dir*    entry = 0;
    UInt        block_number;
    UInt        offset = 0;
    size_t      rsize = 0;

    while (1) {
        memset(buf, 0, PAGE_SIZE);
        if (cd->Read(buf, _partition->BlockSize(), offset, &rsize) != ERR_NONE) {
            return 0;
        }

        if (rsize == 0) {
            // Allocate a new data block
            block_number = _partition->AllocateDataBlock(cd->Ino(), 1);
            if (block_number == 0) {
                return 0;
            }
        }

        // Try allocating a new entry in the block
        entry = AllocateDirEntryHelper(reinterpret_cast<Ext2Dir*>(buf),
                                       name_len);
        if (entry != 0) {
            goto exit;
        }

        offset += _partition->BlockSize();
    }

exit:
    // Update the block with the buffer
    cd->Write(buf, _partition->BlockSize(), offset, 0);

    return entry;
}

/*

stat_t
Ext2Fs::ReleaseDirEntry(Ext2Dir* head, UInt ino)
{
    Ext2Dir*    cur = head;
    Ext2Dir*    prev = 0;
    Ext2Dir*    end = DIR_ENTRY(head, _sblock->BlockSize());

    while (cur < end && cur->recordLength != 0) {
        if (cur->inode == ino) {
            cur->inode = 0;
            if (prev != 0) {
                prev->recordLength += cur->recordLength;
            }
            return ERR_NONE;
        }

        if (cur->inode != 0) {
            prev = cur;
        }
        cur = DIR_ENTRY(cur, cur->recordLength);
    }
    return ERR_NOT_FOUND;
}

void
Ext2Fs::RemoveDirEntry(Ext2Inode* cd, UInt ino)
{
    if ((cd->mode & Ext2Inode::IFMASK) != Ext2Inode::IFDIR) {
        return 0;
    }

    static char buf[PAGE_SIZE];

    for (UInt i = 0; i < cd->blocks; i++) {
        UInt blkno;

        // Get the block number of the i-th data block of the inode
	if (GetDataBlock(cd, i, &blkno) != ERR_NONE) {
            break;
	}

        // The data block is supposed to contain directory entries.
        if (_partition->ReadBlock(buf, blkno) != ERR_NONE) {
            break;
        }

        if (RemoveDirEntry(static_cast<Ext2Dir*>(buf), ino) == ERR_NONE) {
            _partition->WriteBlock(buf, blkno);
            break;
        }
    }
}
*/

Bool
Ext2Fs::Access(Ext2File* dir, const char* name, UInt mode)
{
    Int     ino;
    Bool    result = FALSE;
    ENTER;

    if (SearchFile(dir, name, strlen(name),
                   ((mode & Ext2File::DIRECTORY) != 0), &ino) == ERR_NONE) {
        result = TRUE;
    }
    EXIT;
    return result;
}

Ext2File*
Ext2Fs::Open(Ext2File* dir, const char* name, UInt mode)
{
    static const char*  current_dir = ".";
    Ext2File*   file;
    Ext2Inode   inode;
    Int         ino = 0;
    ENTER;

    if (strlen(name) == 0) {
        name = current_dir;
    }

    if (SearchFile(dir, name, strlen(name),
                   ((mode & Ext2File::DIRECTORY) != 0), &ino) != ERR_NONE) {
        return 0;
    }

    // Check file uid, gid and mode

    _partition->Read(ino, &inode);
    file = new Ext2File(_partition, inode, ino, mode);
    if (file == 0) {
        return 0;
    }

    EXIT;
    return file;
}

char *
Ext2Fs::Strip(char *str, char c)
{
    int i;
    while (*str == c) {
        str++;
    }
    i = strlen(str) - 1;
    while (0 < i && str[i] == c) {
        str[i] = '\0';
        i--;
    }
    return str;
}

UInt
Ext2Fs::GetFileNameIndex(const char *path)
{
    Int i;

    // Skip separators at the end
    i = strlen(path) - 1;
    while (0 < i && path[i] == '/') {
        i--;
    }

    while (0 < i && path[i] != '/') {
        i--;
    }

    return i > 0 ? i + 1 : 0;
}

char*
Ext2Fs::SubString(const char* str, Int begin, Int end)
{
    if (strlen(str) - 1 < static_cast<size_t>(end)) {
        end = strlen(str) - 1;
    }

    size_t  len = end - begin + 1;
    char*   substring = new char[len + 1];

    strncpy(substring, str + begin, len);
    substring[len] = '\0';
    return substring;
}

Bool
Ext2Fs::Access(const char* path, UInt mode)
{
    UInt index;
    Bool result = FALSE;

    while (*path == '/') {
        path++;
    }

    index = GetFileNameIndex(path);
    if (index != 0) {
        char*       base = SubString(path, 0, index - 1);
        char*       name = SubString(path, index, strlen(path) - 1);
        Ext2File*   dir = ChangeDirectory(const_cast<Ext2File*>(Root()), base);

        result = Access(dir, name, mode);

        delete base;
        delete name;
        delete dir;
    }
    else {
        result = Access(const_cast<Ext2File*>(Root()), path, mode);
    }

    return result;
}

Ext2File*
Ext2Fs::Open(const char* path, UInt mode)
{
    Ext2File*   file;
    UInt        index;
    ENTER;

    while (*path == '/') {
        path++;
    }

    index = GetFileNameIndex(path);
    if (index != 0) {
        char*       base = SubString(path, 0, index - 1);
        char*       name = SubString(path, index, strlen(path) - 1);
        Ext2File*   dir = ChangeDirectory(const_cast<Ext2File*>(Root()), base);

        file = Open(dir, name, mode);

        delete base;
        delete name;
        delete dir;
    }
    else {
        DOUT("lookup the root directory\n");
        file = Open(const_cast<Ext2File*>(Root()), path, mode);
    }

    EXIT;
    return file;
}

Ext2File*
Ext2Fs::Create(Ext2File* dir, const char *name, UByte type)
{
    ENTER;

    // Already exists?
    if (Access(dir, name, type)) {
        return Open(dir, name, type);
    }

    // Allocate an inode for the file
    UInt ino = _partition->AllocateInode();

    // Register the name to the ext2 directory.
    // NOTE: dir must be opened in append-mode.
    Ext2Dir* entry = AllocateDirEntry(dir, strlen(name));
    if (entry == 0) {
        _partition->ReleaseInode(ino);
        return 0;
    }

    entry->inode = ino;
    entry->nameLength = strlen(name);
    entry->fileType = type;
    memset(entry->name, 0, ROUND_UP(entry->nameLength, 4));
    strcpy(entry->name, name);

    EXIT;
    return 0;
}

void
Ext2Fs::Remove(Ext2File* file)
{
    ENTER;
    //ReleaseDirEntry(ino);
    _partition->ReleaseInode(file->Ino());
    EXIT;
}

Ext2File*
Ext2Fs::ChangeDirectory(Ext2File* cd, const char* path)
{
    static char dir_name[Ext2Dir::MAX_NAME_LENGTH + 1];
    Ext2Inode   inode;
    Int         ino;
    ENTER;

    if (path[0] == '/') {
        return 0;
    }

    UInt i = 0;
    while (path[i] != '\0') {
        // Copy the directory name
        UInt j = 0;
        while (path[i] != '/' && path[i] != '\0') {
            dir_name[j] = path[i];
            i++;
            j++;
            if (j > Ext2Dir::MAX_NAME_LENGTH) {
                return 0;
            }
        }
        dir_name[j] = '\0';

        SearchFile(cd, dir_name, strlen(dir_name), true, &ino);
        _partition->Read(ino, &inode);

        while (path[i] == '/') {
            i++;
        }
    }

    EXIT;
    return new Ext2File(_partition, inode, ino,
                        Ext2File::READ | Ext2File::DIRECTORY);
}

