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
/// @file   Services/Devices/Audio/SB16/dma.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  April 2008
/// 

//$Id: dma.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/IO.h>
#include <Types.h>
#include "dma.h"

static const unsigned int DMA_MASK_REG_8 = 0x0a;
static const unsigned int DMA_MASK_REG_16 = 0xd4;
static const unsigned int DMA_FLIPFLOP_REG_8 = 0xc0;
static const unsigned int DMA_FLIPFLOP_REG_16 = 0xd8;
static const unsigned int DMA_MODE_REG_8 = 0x0b;
static const unsigned int DMA_MODE_REG_16 = 0xd6;

static const unsigned int DMA_ADDRESS_REG[] = { 0x00, 0x02, 0x04, 0x06, 0xc0, 0xc4, 0xc8, 0xcc };
static const unsigned int DMA_COUNT_REG[] = { 0x01, 0x03, 0x05, 0x07, 0xc2, 0xc6, 0xca, 0xce };
static const unsigned int DMA_PAGE_REG[] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };

stat_t
dma_prepare_transfer(UByte channel, DMATransferMode mode,
                     DMATransferDirection direction,
                     addr_t address, size_t size)
{
    Bool is16bits = channel >= 4 ? true : false;

    // Parameter check
    // There is only 8 DMA channels
    if (channel > 7) return ERR_INVALID_ARGUMENTS;
    // The address must be within the first 16Mb of memory
    if (address >= 0x100000) return ERR_INVALID_ARGUMENTS;
    // Size shall not be longer than 65Kb and bigger than 0
    if (size > 0x10000 || size == 0) return ERR_INVALID_ARGUMENTS;
    
    if (is16bits) {
        // Divide address by two if using 16 bits transfer
        address /= 2;
        // Same thing for transfer size - moreover, 1 shall be substracted
        size /= 2;
    }
    // Actual size given to the DMA should be minored by 1
    size -= 1;
    
    // Disable the DMA channel
    outb(is16bits ? DMA_MASK_REG_16 : DMA_MASK_REG_8, 0x4 + (channel % 4));
    
    // Reset the flip-flop
    outb(is16bits ? DMA_FLIPFLOP_REG_16 : DMA_FLIPFLOP_REG_8, 0xff);
    // Issue the mode of DMA transfer
    outb(is16bits ? DMA_MODE_REG_16 : DMA_MODE_REG_8, (channel % 4) | mode | direction);
    
    // Write offset of buffer
    // First low byte
    outb(DMA_ADDRESS_REG[channel], address & 0xff);
    // Then high byte
    outb(DMA_ADDRESS_REG[channel], (address >> 8) & 0xff);
    // Write the page register
    outb(DMA_PAGE_REG[channel], (address >> 16) & 0xff);
    
    // Write count of buffer
    // Low byte
    outb(DMA_COUNT_REG[channel], size & 0xff);
    // High byte
    outb(DMA_COUNT_REG[channel], (size >> 8) & 0xff);
    
    // Enable the DMA channel
    outb(is16bits ? DMA_MASK_REG_16 : DMA_MASK_REG_8, channel % 4);
    
    return ERR_NONE;
}

