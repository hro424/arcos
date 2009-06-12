/*
 *
 *  Copyright (C) 2005, 2006, 2007, 2008, Waseda University.
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
/// @brief  ELF file operations
///
/// @file   Services/P3/Elf.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2005
///

// $Id: Elf.h 349 2008-05-29 01:54:02Z hro $

// Reference:
// [1] "Executable and Linkable Format (ELF)", Portable Formats Specification,
//     Version 1.1 & 1.2, Tool Interface Standards (ITS).

#ifndef ARC_PEL_ELF_H
#define ARC_PEL_ELF_H

#include <String.h>
#include <Types.h>
#include "Elf32.h"

///
/// Check if EHDR is a valid ELF header
///
static inline bool
Elf_InvalidArch(const Elf_Ehdr *e)
{
    // Check the magic numbers
    if (e->e_ident[EI_CLASS] != ELFCLASS32) {
        // invalid arch specific ELF magic (class)
        return true;
    }

    if (e->e_ident[EI_DATA] != ELFDATA2LSB) {
        // invalid arch specific ELF magic (data)
        return true;
    }

    if (e->e_machine != EM_386) {
        // invalid arch specific ELF magic (machine)
        return true;
    }

    return false;
}


///
/// Check if EHDR is a valid ELF header
///
static inline bool
Elf_InvalidHeader(const void *ehdr, unsigned int size)
{
    const Elf_Ehdr *e = (const Elf_Ehdr *)ehdr;

    // Check the header size
    if (size < sizeof(Elf_Ehdr)) {
        // ELF header smaller than expected
        return true;
    }

    if (e->e_ident[EI_MAG0] != ELFMAG0
        || e->e_ident[EI_MAG1] != ELFMAG1
        || e->e_ident[EI_MAG2] != ELFMAG2
        || e->e_ident[EI_MAG3] != ELFMAG3) {
        // invalid architecture indenpendent ELF magic
        return true;
    }

    // Check the magic numbers
    if (Elf_InvalidArch(e)) {
        return true;
    }

    if (e->e_version != EV_CURRENT) {
        // invalid ELF version
        return true;
    }

    if (size < e->e_shoff + e->e_shentsize * e->e_shnum) {
        return true;
    }

    if (size < e->e_phoff + e->e_phentsize * e->e_phnum) {
        return true;
    }

    return false;
}

//----------------------------------------------------------------------
//	Accessors for section headers
//----------------------------------------------------------------------

///
/// Obtains the reference to the section header pointed by the specified index.
///
/// @param ehdr		the ELF object file
/// @param index	the index
/// @return		the section header
///
static inline const Elf_Shdr *
Elf_GetSectionHeader(const Elf_Ehdr *ehdr, Elf_Word index)
{
    if (ehdr == 0 || index > ehdr->e_shnum) {
        return 0;
    }

    return (Elf_Shdr *)((Elf_Addr)ehdr + ehdr->e_shoff +
			ehdr->e_shentsize * index);
}

static inline const Elf_Phdr *
Elf_GetProgramHeader(const Elf_Ehdr *ehdr, Elf_Word index)
{
    if (ehdr == 0 || ehdr->e_shnum < index) {
        return 0;
    }

    return (Elf_Phdr *)((Elf_Addr)ehdr + ehdr->e_phoff +
                        ehdr->e_phentsize * index);
}

///
/// Obtains the reference to the section header of the section name string
/// table.
///
/// @param ehdr		the ELF object file
/// @return		the section header
///
static inline const Elf_Shdr *
Elf_GetStringSectionHeader(const Elf_Ehdr *ehdr)
{
    return Elf_GetSectionHeader(ehdr, ehdr->e_shstrndx);
}

///
/// Obtains the reference to the section header of the type.
///
/// @param ehdr		the ELF oject file
/// @param type		the type of the section
/// @return		the section header
///
static inline const Elf_Shdr *
Elf_GetSectionHeaderByType(const Elf_Ehdr *ehdr, Elf_Word type)
{
    Elf_Addr    cur;
    int         i;

    if (ehdr != 0) {
        cur = (Elf_Addr)ehdr + ehdr->e_shoff;
        for (i = 0; i < ehdr->e_shnum; ++i) {
            if (((Elf_Shdr *)cur)->sh_type == type) {
                return (Elf_Shdr *)cur;
            }
            cur += ehdr->e_shentsize;
        }
    }
    return 0;
}

///
/// Obtains the reference to the section header of the symbol table. According
/// to [1], an object file may have only one symbol table section, but this
/// restriction may be relaxed in the future.
/// @param ehdr		the ELF object file
/// @return		the section header of the symbol table
///
static inline const Elf_Shdr *
Elf_GetSymbolSectionHeader(const Elf_Ehdr *ehdr)
{
    return Elf_GetSectionHeaderByType(ehdr, SHT_SYMTAB);
}

//----------------------------------------------------------------------
//	Accessors for sections
//----------------------------------------------------------------------

///
/// Obtains the symbol table of the file.
/// @param ehdr		the ELF object file
/// @return		the reference to the symbol table
///
static inline Elf_Addr
Elf_GetSymbolTable(const Elf_Ehdr *ehdr)
{
    const Elf_Shdr  *s = Elf_GetSymbolSectionHeader(ehdr);
    if (s != 0) {
        return (Elf_Addr)ehdr + s->sh_offset;
    }
    return 0UL;
}

///
/// Obtains the symbol name string table of the file.
/// @param ehdr		the ELF object file
/// @return		the reference to the symbol table
///
static inline Elf_Addr
Elf_GetSymbolStringTable(const Elf_Ehdr *ehdr)
{
    const Elf_Shdr  *s1 = Elf_GetSymbolSectionHeader(ehdr);
    if (s1 == 0) {
        return 0UL;
    }
    const Elf_Shdr  *s2 = Elf_GetSectionHeader(ehdr, s1->sh_link);
    if (s2 == 0) {
        return 0UL;
    }
    return (Elf_Addr)ehdr + s2->sh_offset;
}

///
/// Obtains the section name string table of the file.
/// @param ehdr		the ELF object file
/// @return		the reference to the symbol table
///
static inline Elf_Addr
Elf_GetStringTable(const Elf_Ehdr *ehdr)
{
    const Elf_Shdr  *s = Elf_GetStringSectionHeader(ehdr);
    if (s == 0) {
        return 0;
    }
    return (Elf_Addr)ehdr + s->sh_offset;
}


///
/// Finds the first section matched with the query.
/// @param ehdr		the ELF object file
/// @param sh_name	the name of the section
/// @return		the reference to the section
///
static inline const Elf_Shdr *
Elf_FindSectionHeader(const Elf_Ehdr *ehdr, const char *sh_name)
{
    Elf_Shdr    *s;
    char        *str;
    int         i;

    str = (char *)Elf_GetStringTable(ehdr);
    s = (Elf_Shdr *)((char *)ehdr + ehdr->e_shoff);
    for (i = 0; i < ehdr->e_shnum; i++) {
        if (strcmp(str + s->sh_name, sh_name) == 0) {
            return s;
        }
        s = (Elf_Shdr *)((char *)s + ehdr->e_shentsize);
    }
    return 0;
}

///
/// Utility for doing something for each section in an ELF object file.
/// idx:	the index of the section
/// shdr:	the reference to the section header
/// ehdr:	the ELF object file
///
#define Elf_section_foreach(idx, shdr, ehdr) \
    for (idx = 0, shdr = (Elf_Shdr *)((char *)ehdr + ehdr->e_shoff); \
	 idx < ehdr->e_shnum; \
	 idx++, shdr = (Elf_Shdr *)((char *)shdr + ehdr->e_shentsize))

/*
static inline long
Elf_GetLong(Elf_Ehdr *e, const char *sh_name)
{
	Elf_Shdr *s;

	if (NULL == e || NULL == sh_name) {
		return ERR_BAD_ARGUMENTS;
	}
	s = Elf_GetSectionHeader(e, sh_name);
	if (NULL == s) {
		return ERR_NOT_FOUND;
	}

	return *(long *)((char *)e + s->sh_offset);
}
*/

/*
static inline void
Elf_PrintSectionHeader(const Elf_Shdr *s)
{
    ConsoleOut(INFO, "=== Print Section Header (%p)\n", s);
    ConsoleOut(INFO,
               "%lu, %lu, 0x%lX, 0x%lX, %lu, %lu, 0x%lX, 0x%lX, %lu, %lu\n",
               s->sh_name, s->sh_type, s->sh_flags, s->sh_addr,
               s->sh_offset, s->sh_size, s->sh_link, s->sh_info,
               s->sh_addralign, s->sh_entsize);
}
*/

#endif // ARC_PEL_ELF_H
