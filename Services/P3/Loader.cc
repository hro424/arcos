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
/// @brief  Task loader
/// @file   Services/P3/Loader.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Loader.cc 426 2008-10-01 09:38:07Z hro $

#include <Debug.h>
#include <FileStream.h>
#include <Ipc.h>
#include <MemoryAllocator.h>
#include <MemoryManager.h>
#include <Assert.h>
#include <String.h>
#include <Types.h>
#include <LoadInfo.h>
#include <TaskMap.h>
#include <sys/Config.h>
#include <l4/types.h>
#include "Elf.h"
#include "Loader.h"
#include "Pel.h"
#include "Task.h"

extern stat_t Inject(Task* t, UInt type, UInt freq);

stat_t
TaskLoader::Load(Task* t, const char* fs, const char* path, UInt type, UInt freq)
{
    const LoadInfo* info;
    stat_t          err;

    ENTER;

    //
    // Get the information of the user program
    //
    info = Pel::GetLoadInfo();

    DOUT("PEL (%.8lX) is loading '%s' ...\n", L4_Myself().raw, path);

    DOUT("type: %lu\n", info->type);

    DOUT("text: %.8lX (%u), data: %.8lX (%u), entry: %.8lX\n",
         info->task_map.text_start, info->task_map.text_size,
         info->task_map.data_start, info->task_map.data_size,
         info->task_map.entry);
    
    if ((info->type & 0x0F) == LoadInfo::LOAD_INFO_MEM) {
        //
        // Just map the image already on the RAM
        //
        err = Load0(t, &info->task_map);
        if (err != ERR_NONE) {
            return err;
        }
    }
    else if ((info->type & 0x0F) == LoadInfo::LOAD_INFO_MODULE) {
        err = Load0(t, info->task_map.text_start, info->task_map.text_size);
        if (err != ERR_NONE) {
            return err;
        }
    }
    else if ((info->type & 0x0F) == LoadInfo::LOAD_INFO_FILE) {
        //
        // Load the program from a disk
        //
        DOUT("Load %s from %s\n", path, fs);
        err = Load0(t, Pel::GetFs(fs), path);
        if (err != ERR_NONE) {
            DOUT("fs not found\n");
            return err;
        }
    }

    //
    // Initialize the heap and the stack area of the user task
    //
    t->InitMemory();

    // Fault injection
    if (type > 0) {
        System.Print("fault injection type:%d freq:%d\n", type, freq);
        Inject(t, type, freq);
    }

    EXIT;
    return ERR_NONE;
}

stat_t
TaskLoader::Load0(Task *t, const TaskMap *tm)
{
    stat_t      stat;

    ENTER;

    stat = MapImage(tm);
    if (stat != ERR_NONE) {
        return ERR_NONE;
    }

    //
    // Initialize the area for the program code as text segment
    // TODO: That sounds strange.  The text and data sections should have
    // different access rights.
    //
    t->InitText(tm->entry, tm->text_start, tm->text_size);
    t->InitData(tm->data_start, tm->data_size);
    t->InitPm(tm->pm_start, tm->pm_size);

    EXIT;

    return ERR_NONE;
}

stat_t
TaskLoader::Load0(Task *t, L4_ThreadId_t fs, const char *path)
{
    TaskMap     tm;
    stat_t      err;

    err = LoadFile(fs, path, tm);
    if (err != ERR_NONE) {
        return err;
    }

    t->InitText(tm.entry, tm.text_start, tm.text_size);
    t->InitData(tm.data_start, tm.data_size);
    t->InitPm(tm.pm_start, tm.pm_size);

    return ERR_NONE;
}

stat_t
TaskLoader::Load0(Task* t, addr_t module, size_t length)
{
    TaskMap     tm;
    addr_t      program;
    size_t      plen;
    stat_t      err;
    ENTER;

    err = Unarchive(module, length, program, plen);
    if (err != ERR_NONE) {
        return err;
    }

    err = Extract(program, plen, tm);
    if (err != ERR_NONE) {
        return err;
    }

    // Release the pages where the original archive is placed.  Otherwise,
    // the stack overwrites the archive.
    //HRO: quick hack
    //ReleaseTemporaryPages(module, length >> PAGE_BITS + 1);

    DOUT("text@%.8lX (%.8lX) data@%.8lX (%.8lX) pm@%.8lX (%.8lX)\n",
         tm.text_start, tm.text_size, tm.data_start, tm.data_size,
         tm.pm_start, tm.pm_size);

    t->InitText(tm.entry, tm.text_start, tm.text_size);
    t->InitData(tm.data_start, tm.data_size);
    t->InitPm(tm.pm_start, tm.pm_size);

    EXIT;
    return ERR_NONE;
}

stat_t
TaskLoader::MapImage(const TaskMap *tm)
{
    L4_Msg_t        msg;
    L4_Word_t       reg[5];
    ENTER;

    L4_Accept(L4_MapGrantItems(L4_CompleteAddressSpace));

    reg[1] = L4_Myself().raw;
    reg[2] = 0;
    reg[3] = L4_Myself().raw;
    reg[4] = 1;

    //
    // Map the text section
    //
    for (L4_Word_t addr = tm->text_start;
         addr < tm->text_start + tm->text_size;
         addr += PAGE_SIZE) {

        reg[0] = addr & PAGE_MASK;

        L4_Put(&msg, MSG_PAGER_MAP | L4_ReadeXecOnly, 5, reg, 0, 0);
        if (Ipc::Call(L4_Pager(), &msg, &msg) != ERR_NONE) {
            return ERR_UNKNOWN;
        }
    }

    //
    // Map the data section
    //
    for (L4_Word_t addr = tm->data_start;
         addr < tm->data_start + tm->data_size;
         addr += PAGE_SIZE) {

        reg[0] = addr & PAGE_MASK;

        L4_Put(&msg, MSG_PAGER_MAP | L4_ReadWriteOnly, 5, reg, 0, (void *)0);
        if (Ipc::Call(L4_Pager(), &msg, &msg) != ERR_NONE) {
            return ERR_UNKNOWN;
        }
    }

    //
    // Map the persistent section
    //
    for (L4_Word_t addr = tm->pm_start;
         addr < tm->pm_start + tm->pm_size;
         addr += PAGE_SIZE) {
        reg[0] = addr & PAGE_MASK;

        L4_Put(&msg, MSG_PAGER_MAP | L4_ReadWriteOnly, 5, reg, 0, 0);
        stat_t err = Ipc::Call(L4_Pager(), &msg, &msg);
        if (err != ERR_NONE) {
            return err;
        }
    }

    L4_Accept(L4_MapGrantItems(L4_Nilpage));

    EXIT;
    return ERR_NONE;
}

stat_t
TaskLoader::Unarchive(addr_t arch, size_t alen, addr_t& prog, size_t& plen)
{
    static const char*  ARCH_HEADER = "!<arch>\n";
    char*               ptr;
    ENTER;

    if (alen == 0) {
        return ERR_INVALID_ARGUMENTS;
    }

    ptr = reinterpret_cast<char*>(arch);
    if (strncmp(ptr, ARCH_HEADER, 8) != 0) {
        DOUT("invalid header\n");
        return ERR_INVALID_ARGUMENTS;
    }

    ptr = reinterpret_cast<char*>(arch) + 58 + strlen(ARCH_HEADER);
    if (ptr[0] != 0x60 || ptr[1] != 0x0A) {
        DOUT("invalid header 2: %X %X\n", ptr[0], ptr[1]);
        return ERR_INVALID_ARGUMENTS;
    }

    prog = reinterpret_cast<addr_t>(ptr + 2);

    ptr = reinterpret_cast<char*>(arch) + 48 + strlen(ARCH_HEADER);
    size_t size = 0;
    for (Int i = 0; i < 8; i++) {
        if (ptr[i] == ' ') {
            break;
        }
        size *= 10;
        size += ptr[i] - '0';
    }
    plen = size;

    EXIT;
    return ERR_NONE;
}

///
/// Loads an executable file at the path
///
stat_t
TaskLoader::LoadFile(L4_ThreadId_t fs, const char *path, TaskMap& tm)
{
    FileStream      file;
    addr_t          image;
    addr_t          program;
    size_t          len;
    size_t          plen;
    stat_t          err;

    ENTER;

    if ((err = file.Connect(fs)) != ERR_NONE) {
        return err;
    }

    if ((err = file.Open(path, FileStream::READ)) != ERR_NONE) {
        return err;
    }

    //
    // Place the ELF file to a temporary area, then extract it to the
    // pre-defined area.
    //

    len = file.Size();
    DOUT("file size: %u\n", len);
    image = AllocateTemporaryPages(PAGE_ALIGN(len) >> PAGE_BITS);
    file.Read((void *)image, len, 0);

    file.Close();
    file.Disconnect();

    err = Unarchive(image, len, program, plen);
    if (err != ERR_NONE) {
        return err;
    }

    err = Extract(program, plen, tm);

    ReleaseTemporaryPages(image, PAGE_ALIGN(len) >> PAGE_BITS);

    EXIT;
    return err;
}

//XXX:  Currently, it uses the user stack area for the temporary area.  I'm not
//      sure if it's alright.
addr_t 
TaskLoader::AllocateTemporaryPages(size_t count)
{
    addr_t  addr = VirtLayout::USER_STACK_END;
    for (size_t i = 0; i < count; i++) {
        addr -= PAGE_SIZE;
        Pager.Reserve(addr);
    }
    return addr;
}

void
TaskLoader::ReleaseTemporaryPages(addr_t base, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        Pager.Release(base + PAGE_SIZE * i);
    }
}

stat_t
TaskLoader::LoadSegment(const Elf_Ehdr *ehdr, const Elf_Phdr *phdr)
{ 
    stat_t      err;
    L4_Msg_t    msg;
    L4_Word_t   reg[3];
    Elf_Addr    segment = (Elf_Addr)ehdr + phdr->p_offset;

    ENTER;

    // Allocate memory pages
    for (Elf_Word vaddr = phdr->p_vaddr & PAGE_MASK;
         vaddr < (phdr->p_vaddr & PAGE_MASK) + phdr->p_memsz;
         vaddr += PAGE_SIZE) {

        reg[0] = vaddr;
        reg[1] = 1;
        reg[2] = (L4_ThreadNo(L4_Myself()) << 14) | phdr->p_flags;

        L4_Put(&msg, MSG_PAGER_ALLOCATE, 3, reg, 0, 0);
        err = Ipc::Call(L4_Pager(), &msg, &msg);

        if (err != ERR_EXIST && err != ERR_NONE) {
            return err;
        }
    }

    memset((void *)phdr->p_vaddr, 0, phdr->p_memsz);
    memcpy((void *)phdr->p_vaddr, (void *)segment, phdr->p_filesz);

    EXIT;
    return ERR_NONE;
}

void
TaskLoader::ReadSegments(const Elf_Ehdr* header, TaskMap& tm)
{
    const Elf_Phdr* phdr;
    Int             counter = 0;

    for (Int i = 0; i < header->e_phnum; i++) {

        phdr = Elf_GetProgramHeader(header, i);

        if (phdr->p_type == PT_LOAD) {
            // Assume that a segment with executable is text
            if ((phdr->p_flags & PF_W) == 0 &&
                (phdr->p_flags & PF_X) != 0) {
                tm.text_start = phdr->p_vaddr;
                tm.text_size = phdr->p_memsz;
            }
            // Assume that a segment with write is data or persistent
            else if ((phdr->p_flags & PF_W) != 0) {
                if (counter == 0) {
                    tm.data_start = phdr->p_vaddr;
                    tm.data_size = phdr->p_memsz;
                }
                else if (counter == 1){
                    tm.pm_start = phdr->p_vaddr;
                    tm.pm_size = phdr->p_memsz;
                }
                counter++;
            }
        }
    }
}

///
/// Loads the ELF executable
///
stat_t
TaskLoader::Extract(addr_t image, size_t length, TaskMap& tm)
{
    const Elf_Phdr* phdr;
    Elf_Ehdr*       header;

    ENTER;
    assert(image != 0);

    header = reinterpret_cast<Elf_Ehdr *>(image);
    if (Elf_InvalidHeader(header, length)) {
        BREAK("pel: invalid ELF program header");
        return ERR_INVALID_ARGUMENTS;
    }

    if (length < header->e_phoff + header->e_phentsize * header->e_phnum) {
        BREAK("pel: invalid ELF program length");
        return ERR_INVALID_ARGUMENTS;
    }

    if (header->e_phoff == 0 || header->e_type != ET_EXEC) {
        BREAK("pel: binary is not executable");
        return ERR_INVALID_ARGUMENTS;
    }

    ReadSegments(header, tm);

    // Get the information of the persistent memory area
    if (header->e_shoff != 0) {
        const Elf_Shdr* shdr = Elf_FindSectionHeader(header, ".pdata");
        if (shdr != 0) {
            tm.pm_start = shdr->sh_addr;
            tm.pm_size = shdr->sh_size;
            DOUT("PM segment found: base %.8lX size %u\n",
                 tm.pm_start, tm.pm_size);
        }
    }

    // Load segments
    for (int i = 0; i < header->e_phnum; i++) {
        phdr = Elf_GetProgramHeader(header, i);

        if (phdr->p_vaddr == tm.pm_start) {
            DOUT("segment %d is pm.\n", i);
            continue;
        }

        switch (phdr->p_type) {
            case PT_NULL:
                break;
            case PT_LOAD:
                LoadSegment(header, phdr);
                break;
            case PT_DYNAMIC:
            case PT_INTERP:
            case PT_NOTE:
            case PT_SHLIB:
            case PT_PHDR:
            case PT_LOPROC:
            case PT_HIPROC:
            default:
                BREAK("not implemented");
                return ERR_UNKNOWN;
        }
    }

    tm.entry = static_cast<addr_t>(header->e_entry);

    EXIT;
    return ERR_NONE;
}

