/*****************************************************************************
 *
 *  Copyright (C) 2007, 2008, Waseda University
 *  All rights reserved.
 *
 *  @file   Libraries/arc/ia32/lv1.lds
 *  @breif  Linker script for level-1 user tasks
 *  @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
 *  @since  2007
 *
 *  $Id: lv1.lds 374 2008-08-07 05:48:21Z hro $
 */

ENTRY(_start)

PHDRS
{
    init PT_LOAD FLAGS (5) ;
    text PT_LOAD FLAGS (5) ;
    data PT_LOAD ;
    pdata PT_LOAD ;
}

SECTIONS
{
    . = 0x20000000;

    .init   : { *(.init) } : init

    .text   : { *(.text) } : text

    .rodata : { *(.rodata*) }

    .ctors  : { *(.ctors) }

    .dtors  : { *(.dtors) }

    . = ALIGN(4k);

    .data :
    {
        *(.data)
        *(.data.*)

    } : data
    
    .bss :
    {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
    
    . = ALIGN(4k);
    
    .pdata  : { *(.pdata) } : pdata
}

