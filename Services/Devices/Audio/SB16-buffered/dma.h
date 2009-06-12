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
/// @file   Services/Devices/SB16/dma.h
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  April 2008
/// 

//$Id: dma.h 347 2008-05-28 07:56:32Z gnurou $

#ifndef ARC_SB16_DMA_H_
#define ARC_SB16_DMA_H_

#include <Types.h>

typedef enum {
    ONDEMAND = 0x00, SINGLE = 0x40, BLOCK = 0x80,
    CASCADE = 0xc0, AUTOINIT = 0x10, ADRINC = 0x00, ADRDEC = 0x20
} DMATransferMode;

typedef enum {
    TOMEMORY = 0x4, FROMMEMORY = 0x8
} DMATransferDirection;

stat_t dma_prepare_transfer(UByte channel, DMATransferMode mode,
                            DMATransferDirection direction,
                            addr_t address, size_t size);

#endif // ARC_SB16_DMA_H_

