/*
 *
 *  Copyright (C) 2007, Waseda University. All rights reserved.
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
/// @file   Services/Devices/Sata/PhysicalRegionDescriptor.cc
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

// $Id: PhysicalRegionDescriptor.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/types.h>
#include "PhysicalRegionDescriptor.h"

void
PhysicalRegionDescriptor::SetDataBlock(addr_t addr, size_t length)
{
    _base = (addr & (~0UL - 1));
    _baseUpper = 0;
    //_baseUpper = (PTR)(addr >> 32);

    _count = (length & BYTE_COUNT_MASK) - 1;
}

void
PhysicalRegionDescriptor::UnsetDataBlock() {
    _base = 0;
    _count = (_count & INTERRUPT_COMPLETION) | 1;
}


addr_t
PhysicalRegionDescriptor::BaseAddress() {
    addr_t   addr = _base;
    //addr |= (uint64_t)_baseUpper << 32;
    return addr;
}

size_t
PhysicalRegionDescriptor::Length()
{
    return (_count + 1) & BYTE_COUNT_MASK;
}

void
PhysicalRegionDescriptor::SetInterrupt()
{
    _count |= INTERRUPT_COMPLETION;
}

void
PhysicalRegionDescriptor::ClearInterrupt()
{
    _count &= ~INTERRUPT_COMPLETION;
}

bool
PhysicalRegionDescriptor::IsInterrupt()
{
    return (_count & INTERRUPT_COMPLETION) == INTERRUPT_COMPLETION;
}


