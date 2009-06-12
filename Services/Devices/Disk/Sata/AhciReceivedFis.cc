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
/// @file   Services/Devices/Sata/AhciReceivedFis.cc
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @date   2007
///

//$Id: AhciReceivedFis.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/status.h>
#include <arc/types.h>
#include "AhciReceivedFis.h"

status_t
AhciReceivedFis::Initialize(addr_t receivedFis)
{
    if ((receivedFis & (LENGTH - 1)) > 0) {
        // not aligned to 256
        return ERR_INVALID_ARGUMENTS;
    }

    base = receivedFis;
    dmaSetup = receivedFis;
    pioSetup = receivedFis + PSFIS_OFFSET;
    d2hRegister = receivedFis + DSFIS_OFFSET;
    deviceBits = receivedFis + SDBFIS_OFFSET;
    unknown = receivedFis + UFIS_OFFSET;

    Clear();

    return ERR_NONE;
}

void
AhciReceivedFis::Clear()
{
    for (UByte *ptr = (UByte *)base; ptr < (UByte *)base + LENGTH; ptr++) {
        *ptr = 0;
    }
}

