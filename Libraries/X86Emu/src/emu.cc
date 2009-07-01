/*
 *
 *  Copyright (C) 2008, Waseda University.
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
/// @file   Services/x86emu/lib/Arc/emu.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// Defines the Arc stubs for the x86emu real-mode emulator.

#include <x86emu/x86emu.h>
#include <x86emu/arcstubs.h>
#include <l4/types.h>

static u8 my_inb(X86EMU_pioAddr addr) X86API;
static u16 my_inw(X86EMU_pioAddr addr) X86API;
static u32 my_inl(X86EMU_pioAddr addr) X86API;
static void my_outb(X86EMU_pioAddr addr, u8 val) X86API;
static void my_outw(X86EMU_pioAddr addr, u16 val) X86API;
static void my_outl(X86EMU_pioAddr addr, u32 val) X86API;

static u8 my_rdb(u32 addr) X86API;
static u16 my_rdw(u32 addr) X86API;
static u32 my_rdl(u32 addr) X86API;
static void my_wrb(u32 addr, u8 val) X86API;
static void my_wrw(u32 addr, u16 val) X86API;
static void my_wrl(u32 addr, u32 val) X86API;

/**
 * Pointers to the functions handling port input/output
 */
X86EMU_pioFuncs my_pioFuncs =
{
	my_inb, my_inw, my_inl,
	my_outb, my_outw, my_outl
};

/**
 * Pointers to the functions handling memory input/output
 */
X86EMU_memFuncs my_memFuncs =
{
	my_rdb, my_rdw, my_rdl,
	my_wrb, my_wrw, my_wrl
};

extern L4_Word8_t *main_mem;

static u8 X86API
my_inb(X86EMU_pioAddr addr)
{
	int r;
	asm volatile("inb %w1, %b0" : "=a" (r) : "d" (addr));
	return r;
}

static u16 X86API
my_inw(X86EMU_pioAddr addr)
{
	int r;
	asm volatile("inw %w1, %w0" : "=a" (r) : "d" (addr));
	return r;
}

static u32 X86API
my_inl(X86EMU_pioAddr addr)
{
	int r;
	asm volatile("inl %w1, %0" : "=a" (r) : "d" (addr));
	return r;
}

static void X86API
my_outb(X86EMU_pioAddr addr, u8 val)
{
  asm volatile("outb %b0, %w1" : "=a" (val), "=d" (addr)
		  : "a" (val), "d" (addr));
}

static void X86API
my_outw(X86EMU_pioAddr addr, u16 val)
{
  asm volatile("outw %w0, %w1" : "=a" (val), "=d" (addr)
		  : "a" (val), "d" (addr));
}

static void X86API
my_outl(X86EMU_pioAddr addr, u32 val)
{
  asm volatile("outl %0, %w1" : "=a"(val), "=d" (addr)
		  : "a" (val), "d" (addr));
}

static u8 X86API
my_rdb(u32 addr)
{
	return *(u8*)(main_mem + addr);
}

static u16 X86API
my_rdw(u32 addr)
{
	return *(u16*)(main_mem + addr);
}

static u32 X86API
my_rdl(u32 addr)
{
	return *(u32*)(main_mem + addr);
}

static void X86API
my_wrb(u32 addr, u8 val)
{
	*(u8*)(main_mem + addr) = val;
}

static void X86API
my_wrw(u32 addr, u16 val)
{
	*(u16*)(main_mem + addr) = val;
}

static void X86API
my_wrl(u32 addr, u32 val)
{
	*(u32*)(main_mem + addr) = val;
}
