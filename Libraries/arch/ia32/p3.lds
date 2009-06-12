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

/*
 *  @file   Libraries/arch/ia32/linker.lds
 *  @breif  Linker script for P3
 *  @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
 *  @since  December 2007
 *
 *  $Id: p3.lds 349 2008-05-29 01:54:02Z hro $
 */

ENTRY(_start)

/* Physical address of the beginning of the text section */
_start_text_phys = 0x00380000;

/* Virtual address of the beginning of the text section */
_start_text = 0x08000000;

_offset = _start_text - _start_text_phys;

/* Command for the ELF program header */
PHDRS
{
    text PT_LOAD FLAGS (5) ;    /* Loadable and executable */
    data PT_LOAD ;              /* Loadable */
}

SECTIONS
{
    .init _start_text : AT (ADDR(.init) - _offset)
    {
        *(.init)
    } : text

    .text . : AT (ADDR(.text) - _offset)
    {
        /* Place the start section on the head */
        *(.start)
        *(.text)
    } : text

    .fini . : AT (ADDR(.fini) - _offset)
    {
        *(.fini)
    } : text

    .rodata . : AT (ADDR(.rodata) - _offset)
    {
        *(.rodata*)
    }

    .ctors . : AT (ADDR(.ctors) - _offset)
    {
        *(.ctors)
    }

    .dtors . : AT (ADDR(.dtors) - _offset)
    {
        *(.dtors)
    }

    /* Align in page size to set different access privileges to data. */
    . = ALIGN(4k);

    .data . : AT (ADDR(.data) - _offset)
    {
        *(.data)
        *(.data.*)
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    } : data
}

