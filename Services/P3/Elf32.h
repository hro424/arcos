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
/// @file    Services/P3/Elf32.h
/// @author  Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since   2005
///

// $Id: Elf32.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_PEL_ELF32_H
#define ARC_PEL_ELF32_H

//
// Reference:
// Tool Interface Standards (TIS), "Executable and Linkable Format (ELF)",
// Portable Formats Specification, Version 1.1 & 1.2.
//

//--------------------------------------------------------------------------
//	Data Representation
//--------------------------------------------------------------------------

typedef unsigned long           Elf32_Addr;     // Unsigned program address
typedef unsigned short          Elf32_Half;     // Unsigned medium integer
typedef unsigned long           Elf32_Off;      // Unsigned file offset
typedef long                    Elf32_Sword;    // Signed large integer
typedef unsigned long           Elf32_Word;     // Unsigned large integer

//--------------------------------------------------------------------------
//	ELF Header
//--------------------------------------------------------------------------

// Size of Elf32_Ehdr::e_ident
#define EI_NIDENT	16

// Indixes for e_ident elements
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4           // File's class or capacity
#define EI_DATA		5           // Data encoding
#define EI_VERSION	6           // ELF header version number
#define EI_PAD		7           // Unused bytes


typedef struct {
    unsigned char       e_ident[EI_NIDENT];
    Elf32_Half          e_type;
    Elf32_Half          e_machine;
    Elf32_Word          e_version;
    Elf32_Addr          e_entry;
    Elf32_Off           e_phoff;
    Elf32_Off           e_shoff;
    Elf32_Word          e_flags;
    Elf32_Half          e_ehsize;
    Elf32_Half          e_phentsize;
    Elf32_Half          e_phnum;
    Elf32_Half          e_shentsize;
    Elf32_Half          e_shnum;
    Elf32_Half          e_shstrndx;
} Elf32_Ehdr;

// Values for e_type
#define ET_NONE		0U          // No file type
#define ET_REL		1U          // Relocatable file
#define ET_EXEC		2U          // Executable file
#define ET_DYN		3U          // Shared object file
#define ET_CORE		4U          // Core file
#define ET_LOPROC	0xff00      // Processor-specific
#define ET_HIPROC	0xffff      // Processor-specific

// Values for e_machine
#define EM_NONE		0U
#define EM_M32		1U
#define EM_SPARC	2U
#define EM_386		3U
#define EM_68K		4U
#define EM_88K		5U
#define EM_860		7U
#define EM_MIPS		8U

// Values for e_version
#define EV_NONE		0U
#define EV_CURRENT	1U

//--------------------------------------------------------------------------
//	ELF Identification
//-------------------------------------------------------------------------- 
#define ELFMAG0		0x7f
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

#define ELFCLASSNONE	0           // Invalid class
#define ELFCLASS32	1           // 32-bit objects
#define ELFCLASS64	2           // 64-bit objects

#define ELFDATANONE	0           // Invalid data encoding
#define ELFDATA2LSB	1           // Little endian
#define ELFDATA2MSB	2`          // Big endian

//--------------------------------------------------------------------------
//	Sections
//--------------------------------------------------------------------------

typedef struct {
    Elf32_Word          sh_name;
    Elf32_Word          sh_type;
    Elf32_Word          sh_flags;
    Elf32_Addr          sh_addr;
    Elf32_Off           sh_offset;
    Elf32_Word          sh_size;
    Elf32_Word          sh_link;
    Elf32_Word          sh_info;
    Elf32_Word          sh_addralign;
    Elf32_Word          sh_entsize;
} Elf32_Shdr;

//
//	Special Section Indexes
//
#define SHN_UNDEF	0x0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_HIRESERVE	0xffff

//
//	Section Types
//
#define SHT_NULL	0UL
#define SHT_PROGBITS	1UL
#define SHT_SYMTAB	2UL
#define SHT_STRTAB	3UL
#define SHT_RELA	4UL
#define SHT_HASH	5UL
#define SHT_DYNAMIC	6UL
#define SHT_NOTE	7UL
#define SHT_NOBITS	8UL
#define SHT_REL		9UL
#define SHT_SHLIB	10UL
#define SHT_DYNSYM	11UL
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

//
//	Section Attribute Flags
//

// The section contains data that should be writable during process execution.
#define SHF_WRITE	0x1
// The section occupies memory during process execution.
#define SHF_ALLOC	0x2
// The section contains executable machine instructions.
#define SHF_EXECINSTR	0x4
// All bits included in this mask are reserved for processor-specific
// semantics.
#define SHF_MASKPROC	0xf0000000

//--------------------------------------------------------------------------
//	Symbol Table
//--------------------------------------------------------------------------

typedef struct Elf32_symbol_table {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} Elf32_Sym;

#define ELF32_ST_BIND(i)	((i)>>4)
#define ELF32_ST_TYPE(i)	((i)&0xf)
#define ELF32_ST_INFO(b,t)	(((b)<<4)+((t)&0xf))

#define STB_LOCAL	0U
#define STB_GLOBAL	1U
#define STB_WEAK	2U
#define STB_LOPROC	13U
#define STB_HIPROC	15U

#define STT_NOTYPE	0U
#define STT_OBJECT	1U
#define STT_FUNC	2U
#define STT_SECTION	3U
#define STT_FILE	4U
#define STT_LOPROC	13U
#define STT_HIPROC	15U

//--------------------------------------------------------------------------
//	Relocation
//--------------------------------------------------------------------------

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i)		((i)>>8)
#define ELF32_R_TYPE(i)		((unsigned char)(i))
#define ELF32_R_INFO(s,t)	(((s)<<8)+(unsigned char)(t))

#define R_386_NONE	0U
#define R_386_32	1U
#define R_386_PC32	2U
#define R_386_GOT32	3U
#define R_386_PLT32	4U
#define R_386_COPY	5U
#define R_386_GLOB_DAT	6U
#define R_386_JMP_SLOT	7U
#define R_386_RELATIVE	8U
#define R_386_GOTOFF	9U
#define R_386_GOTPC	10U


//--------------------------------------------------------------------------
//      Program Header
//--------------------------------------------------------------------------

typedef struct {
    Elf32_Word          p_type;
    Elf32_Off           p_offset;
    Elf32_Addr          p_vaddr;
    Elf32_Addr          p_paddr;
    Elf32_Word          p_filesz;
    Elf32_Word          p_memsz;
    Elf32_Word          p_flags;
    Elf32_Word          p_align;
} Elf32_Phdr;

// Segment types, p_type
#define PT_NULL         0UL
#define PT_LOAD         1UL
#define PT_DYNAMIC      2UL
#define PT_INTERP       3UL
#define PT_NOTE         4UL
#define PT_SHLIB        5UL
#define PT_PHDR         6UL
#define PT_LOPROC       0x70000000
#define PT_HIPROC       0x7FFFFFFF

// Segment Flag Bits, p_flags
#define PF_X            0x1
#define PF_W            0x2
#define PF_R            0x4
#define PF_MASKPROC     0xF0000000


typedef Elf32_Word		Elf_Word;
typedef Elf32_Half		Elf_Half;
typedef Elf32_Addr		Elf_Addr;
typedef Elf32_Ehdr		Elf_Ehdr;
typedef Elf32_Shdr		Elf_Shdr;
typedef Elf32_Sym		Elf_Sym;
typedef Elf32_Rel		Elf_Rel;

#define ELF_ST_BIND(val)	ELF32_ST_BIND(val)
#define ELF_ST_TYPE(val)	ELF32_ST_TYPE(val)
#define ELF_R_SYM(val)		ELF32_R_SYM(val)
#define ELF_R_TYPE(val)		ELF32_R_TYPE(val)
#define ELF_R_INFO(val)		ELF32_R_INFO(val)

typedef Elf32_Ehdr              Elf_Ehdr;
typedef Elf32_Shdr              Elf_Shdr;
typedef Elf32_Phdr              Elf_Phdr;

#endif // ARC_PEL_ELF32_H

