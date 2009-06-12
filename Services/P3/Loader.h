/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @brief  Program loader
/// @file   Services/Pel2/Loader.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Loader.h 417 2008-09-10 05:41:59Z hro $

#ifndef ARC_PEL2_LOADER_H
#define ARC_PEL2_LOADER_H

#include <Types.h>
#include <TaskMap.h>
#include <l4/types.h>
#include "Elf.h"
#include "Task.h"

class TaskLoader
{
protected:
    stat_t MapImage(const TaskMap *tm);

    stat_t LoadFile(L4_ThreadId_t fs, const char *path, TaskMap& tm);

    ///
    /// Extracts the AR archived file.
    ///
    /// @param arch         the archive file
    /// @param alen         the length of the archive file
    /// @param prog         the first file in the archive
    /// @param plen         the length of the first file
    ///
    stat_t Unarchive(addr_t arch, size_t alen, addr_t& prog, size_t& plen);

    addr_t AllocateTemporaryPages(size_t count);

    void ReleaseTemporaryPages(addr_t base, size_t count);

    ///
    /// Extracts the ELF executable file.
    ///
    stat_t Extract(addr_t image, size_t length, TaskMap& tm);

    void ReadSegments(const Elf_Ehdr* ehdr, TaskMap& tm);

    stat_t LoadSegment(const Elf_Ehdr *ehdr, const Elf_Phdr *phdr);

    ///
    /// Loads the program from the file system into the Task object.
    ///
    stat_t Load0(Task *t, L4_ThreadId_t fs, const char *path);

    ///
    /// Loads the ELF file on the memory into the Task object
    ///
    stat_t Load0(Task *t, addr_t image, size_t length);

    ///
    /// Loads the program into the Task object according to the task map
    ///
    stat_t Load0(Task *t, const TaskMap *tm);

public:
    stat_t Load(Task *t, const char* fs, const char* path,
                UInt type, UInt freq);
};

#endif // ARC_PEL2_LOADER_H

