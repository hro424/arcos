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
/// @file   Libraries/arch/ia32/crt0-p3.S
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

// $Id: crt0-p3.S 389 2008-08-30 07:15:53Z hro $

    .global _start
    .global _artid
    .global _load_info

//
// PEL start-up routine
//

    .section .start

_start:
    // Get ART tid
    popl    %eax
    movl    %eax, _artid

    //
    // Copy LoadInfo
    // Copy data from the stack until the length becomes zero
    //

    // Get the length of LoadInfo
    // len = pop()
    popl    %eax
    movl    %eax, %ebx

    // i = 0
    movl    $0, %ecx
1:
    // v = pop()
    popl    %eax

    // *((word_t*)_load_info + i * 4) = v
    movl    %eax, _load_info(,%ecx,4)

    // i++
    addl    $1, %ecx

    // len -= 4
    subl    $4, %ebx

    // if (len == 0) break
    cmpl    $0, %ebx
    jne     1b

    call    _init
    call    _Z10__init_pelPv
    call    main
    push    %eax
    call    _Z6__exiti

    .section .data

_artid:
    .long 0

_load_info:
    .space 512, 0
