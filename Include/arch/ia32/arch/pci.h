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
/// @file   Include/arc/arch/ia32/PCI.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.ac.jp>
/// @date   2006
///

// $Id: pci.h 325 2008-05-09 04:22:15Z hro $

/*
 * Reference:
 * [1]	Intel 440FX PCIset, Intel corporation, May 1996.
 * [2]	82371FB (PIIX) and 82371SB (PIIX3) PCI ISA IDE Xcelerator, Intel
 * 	corporation, April 1997.
 */

#ifndef ARC_IA32_DEVICE_PCI_H
#define ARC_IA32_DEVICE_PCI_H

#include <Types.h>

///
/// Reads a byte of data in the specified register in the configuration data
/// register.
///
/// @param address   the address of the device (bus, dev, func)
/// @param rn        the register number
///
static inline UByte
PCI_Read8(unsigned int address, unsigned int rn)
{
    UInt    data;
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((rn & 0xFF) & ~3U)));
    data = inb(IO_PCI_CONFDATA | (rn & 3U));
    outl(IO_PCI_CONFADD, address);
    return data;
}

///
/// Reads a word of data in the specified register in the configuration data
/// register.
///
static inline UShort
PCI_Read16(unsigned int address, unsigned int rn)
{
    UInt    data;
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((rn & 0xFF) & ~3U)));
    data = inw(IO_PCI_CONFDATA | (rn & 2U));
    outl(IO_PCI_CONFADD, address);
    return data;
}

///
/// Reads a double-word of data in the specified register in the configuration
/// data register.
///
static inline UInt
PCI_Read32(unsigned int address, unsigned int rn)
{
    UInt    data;
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((rn & 0xFF) & ~3U)));
    data = inl(IO_PCI_CONFDATA);
    outl(IO_PCI_CONFADD, address);
    return data;
}

///
/// Writes a byte of data into the specified PCI configuration data register.
///
static inline void
PCI_Write8(unsigned int address, unsigned int reg, UByte val)
{
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((reg & 0xFF) & ~3U)));
    outb((IO_PCI_CONFDATA | (reg & 3U)), val);
    outl(IO_PCI_CONFADD, address);
}

///
/// Writes a word of data into the specified PCI configuration data register.
///
static inline void
PCI_Write16(unsigned int address, unsigned int reg, UShort val)
{
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((reg & 0xFF) & ~3U)));
    outw((IO_PCI_CONFDATA | (reg & 3U)), val);
    outl(IO_PCI_CONFADD, address);
}

///
/// Writes a double-word of data into the specified PCI configuration data
/// register.
///
static inline void
PCI_Write32(unsigned int address, unsigned int reg, UInt val)
{
    outl(IO_PCI_CONFADD, (CONFADD_CFGE | address | ((reg & 0xFF) & ~3U)));
    outl((IO_PCI_CONFDATA | (reg & 3U)), val);
    outl(IO_PCI_CONFADD, address);
}

#endif // ARC_IA32_DEVICE_PCI_H

