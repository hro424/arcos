/*****************************************************************************
 *
 *  Copyright (C) 2007, 2008, Waseda University
 *  All rights reserved.
 *
 *  @file   Libraries/Standard/lds/init.lds
 *  @breif  Linker script for user tasks
 *  @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
 *  @since  2007
 *
 *  $Id: lv0.lds 349 2008-05-29 01:54:02Z hro $
 */

ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS (5) ;
    data PT_LOAD ;
    pdata PT_LOAD ;
}

SECTIONS
{
    .init   :
    {
        *(.init)
    } : text

    .text   :
    {
        *(.text)
    } : text

    .fini   :
    {
        *(.fini)
    } : text

    .rodata : { *(.rodata*) }

    .ctors  :
    {
        *(.ctors)
    }

    .dtors  :
    {
        *(.dtors)
    }

    .data   :
    {
        *(.data)
        *(.data.*)

    } : data
    
    .bss    :
    {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
}

