/*********************************************************************
 *                
 * Copyright (C) 1999-2003,  Karlsruhe University
 *                
 * File path:     l4test/amd64/crt0.S
 * Description:   Startup code for x86 family processors.
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: crt0.S,v 1.3 2006/10/19 22:57:40 ud3 Exp $
 *                
 ********************************************************************/
#include <config.h>

	.text
	.global _start
	.global _stext
_stext:	
_start:
	leaq	stack, %rsp
	pushq	%rax
	callq	__L4_Init
	pushq	$___return_from_main
	jmp	main

#if 1
	.align 16, 0x90
__mb_header:
	.long   0x1BADB002;
	.long   0x00010000;
	.long   - 0x00010000 - 0x1BADB002;
	.long   __mb_header;
	.long   _start;
	.long   _edata;
	.long   _end;
	.long   _start;
#endif
	
___return_from_main:
	int $3
	jmp 1f
	.ascii "System stopped."
1:	jmp ___return_from_main
		
	.bss

	.space	1024
stack:
