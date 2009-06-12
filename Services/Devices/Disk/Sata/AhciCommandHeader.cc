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
/// @file   Services/Devices/Sata/AhciCommandHeader.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: AhciCommandHeader.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/status.h>
#include <arc/types.h>
#include "AhciCommandHeader.h"

void
AhciCommandHeader::SetPrdtl(size_t length)
{
    _head &= ~PRDTL_MASK;
    _head |= (UInt)length << 16;
}

UInt
AhciCommandHeader::Prdtl()
{
    return (UInt)(_head >> 16);
}

void
AhciCommandHeader::SetPmp(UInt port)
{
    _head &= ~PMP_MASK;
    _head |= (UInt)port << 12;
}

UInt
AhciCommandHeader::Pmp()
{
    return (UInt)((_head >> 12) & 0xF);
}

void
AhciCommandHeader::SetClearStatus()
{
    _head |= CLEAR_STATUS;
}

void
AhciCommandHeader::ClearClearStatus()
{
    _head &= ~CLEAR_STATUS;
}

Bool
AhciCommandHeader::IsClearStatus()
{
    return (_head & CLEAR_STATUS) == CLEAR_STATUS;
}

void
AhciCommandHeader::SetBist()
{
    _head |= BIST;
}

void
AhciCommandHeader::ClearBist()
{
    _head &= ~BIST;
}

Bool
AhciCommandHeader::IsBist()
{
    return (_head & BIST) == BIST;
}

void
AhciCommandHeader::SetReset()
{
    _head |= RESET;
}

void
AhciCommandHeader::ClearReset()
{
    _head &= ~RESET;
}

Bool
AhciCommandHeader::IsReset()
{
    return (_head & RESET) == RESET;
}

void
AhciCommandHeader::SetPrefetchable()
{
    _head |= PREFETCHABLE;
}

void
AhciCommandHeader::ClearPrefetchable()
{
    _head &= ~PREFETCHABLE;
}

Bool
AhciCommandHeader::IsPrefetchable()
{
    return (_head & PREFETCHABLE) == PREFETCHABLE;
}

void
AhciCommandHeader::SetDeviceWrite()
{
    _head |= DEVICE_WRITE;
}

void
AhciCommandHeader::ClearDeviceWrite()
{
    _head &= ~DEVICE_WRITE;
}

Bool
AhciCommandHeader::IsDeviceWrite()
{
    return (_head & DEVICE_WRITE) == DEVICE_WRITE;
}

void
AhciCommandHeader::SetAtapiCommand()
{
    _head |= ATAPI_COMMAND;
}

void
AhciCommandHeader::ClearAtapiCommand()
{
    _head &= ~ATAPI_COMMAND;
}

Bool
AhciCommandHeader::IsAtapiCommand()
{
    return (_head & ATAPI_COMMAND) == ATAPI_COMMAND;
}

void
AhciCommandHeader::SetCommandFisLength(size_t length)
{
    _head &= ~CFL_MASK;
    _head |= (UInt)length;
}

UInt
AhciCommandHeader::CommandFisLength()
{
    return (UInt)(_head & CFL_MASK);
}

void
AhciCommandHeader::SetByteCount(UInt c)
{
    _count = c;
}

UInt
AhciCommandHeader::ByteCount()
{
    return _count;
}

void
AhciCommandHeader::SetCommandTable(addr_t addr)
{
    _base = (UInt)addr;
    _baseUpper = 0;
}

AhciCommandTable *
AhciCommandHeader::CommandTable()
{
    addr_t addr = _base;
    return (AhciCommandTable *)addr;
}

