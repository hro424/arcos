/*
 *
 *  Copyright (C) 2006, 2007, Waseda University.
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
/// @file   Include/arc/arch/ia32/io.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2006
///
// $Id: io.h 325 2008-05-09 04:22:15Z hro $

/*
 * Reference:
 * [1]	Intel 440FX PCIset, Intel Corporation, May 1996.
 * [2]	VS440FX Motherboard Technical Product Specification, Intel
 * 	Corporation, October 1996.
 * [3]	82371FB (PIIX) and 82371SB (PIIX3) PCI ISA IDE Xcelerator, Intel
 * 	Corporation, April 1997.
 * [4]	Peter T. McLean. AT Attachment-3 Interface (ATA-3), X3T13, Januray
 * 	1997.
 * [5]	Peter T. McLean. AT Attachment with Packet Interface - 6
 * 	(ATA/ATAPI-6), T13, February 2002.
 */

#ifndef ARC_IA32_IO_H
#define ARC_IA32_IO_H

#include <Types.h>

static inline void
outb(UShort port, UByte value)
{
    __asm__ __volatile__("outb %0, %%dx" : :"a"(value), "d"(port));
}

static inline void
outw(UShort port, UShort value)
{
    __asm__ __volatile__("outw %0, %%dx" : :"a"(value), "d"(port));
}

static inline void
outl(UShort port, UInt value)
{
    __asm__ __volatile__("outl %0, %%dx" : :"a"(value), "d"(port));
}

static inline UByte
inb(UShort port)
{
    UByte tmp;
    __asm__ __volatile__("inb %%dx, %0" :"=a"(tmp) :"d"(port));
    return tmp;
}

static inline UShort
inw(UShort port)
{
    UShort tmp;
    __asm__ __volatile__("inw %%dx, %0" :"=a"(tmp) :"d"(port));
    return tmp;
}

static inline UInt
inl(UShort port)
{
    UInt tmp;
    __asm__ __volatile__("inl %%dx, %0" :"=a"(tmp) :"d"(port));
    return tmp;
}

//
// Reads data from the I/O port. Unit of transfer is byte (8 bits).
//
// @param port	the port to read.
// @param buf	the buffer where the data is stored.
// @param count the size of the buffer in byte.
//
static inline int
IO_Read8(UShort port, void *buf, int count)
{
    int     i;
    UByte   *ptr;

    ptr = (UByte *)buf;
    for (i = 0; i < count; i++) {
        ptr[i] = inb(port);
    }

    return i;
}

//
// Reads data from the I/O port. Unit of transfer is word (16 bits).
//
// @param port      the port to read.
// @param buf       the buffer where the data is stored.
// @param count     the size of the buffer in byte.
//
static inline int
IO_Read16(UShort port, void *buf, int count)
{
    int     i, cnt;
    UShort  *ptr;

    ptr = (UShort *)buf;
    cnt = count / sizeof(UShort);
    for (i = 0; i < cnt; i++) {
        ptr[i] = inw(port);
    }

    return i * sizeof(UShort);
}

//
// Reads data from the I/O port. Unit of transfer is double-word (32 bits).
//
// @param port	the port to read.
// @param buf	the buffer where the data is stored.
// @param count the size of the buffer in byte.
//
static inline int
IO_Read32(UShort port, void *buf, int count)
{
    int     i, cnt;
    UInt    *ptr;

    ptr = (UInt *)buf;
    cnt = count / sizeof(UInt);
    for (i = 0; i < cnt; i++) {
        ptr[i] = inl(port);
    }

    return i * sizeof(UInt);
}

static inline int
IO_Write8(UShort port, const void *buf, int count)
{
    int     i;
    UByte   *ptr;

    ptr = (UByte *)buf;
    for (i = 0; i < count; i++) {
        outb(port, ptr[i]);
    }

    return i;
}

static inline int
IO_Write16(UShort port, const void *buf, int count)
{
    int     i, cnt;
    UShort  *ptr;

    ptr = (UShort *)buf;
    cnt = (count + sizeof(UShort) - 1) / sizeof(UShort);
    for (i = 0; i < cnt; i++) {
        outw(port, ptr[i]);
    }
    return i * sizeof(UShort);
}

static inline int
IO_Write32(UShort port, const void *buf, int count)
{
    int     i, cnt;
    UInt    *ptr;

    ptr = (UInt *)buf;
    cnt = (count + sizeof(UInt) - 1) / sizeof(UInt);
    for (i = 0; i < cnt; i++) {
        outl(port, ptr[i]);
    }
    return i * sizeof(UInt);
}

#endif // ARC_IA32_IO_H

