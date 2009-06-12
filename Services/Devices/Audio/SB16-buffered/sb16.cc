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
/// @file   Services/Devices/SB16/sb16.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  April 2008
/// 

//$Id: sb16.cc 352 2008-05-29 07:33:46Z gnurou $

#include <Debug.h>
#include <Driver.h>
#include <Ipc.h>
#include <MemoryManager.h>
#include <System.h>
#include <arc/IO.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <String.h>

#include "dma.h"
#include <audio/Audio.h>
#include <audio/protocol.h>

// Base I/O port address
static const UInt SB_BASE_IO = 0x220;

// Interupt vector
static const UInt SB_IRQ = 5;

static const UInt SB_DMA1 = 1;
static const UInt SB_DMA2 = 5;

static const UByte SB_DSP_RESET = 0x6;
static const UByte SB_DSP_READ = 0xa;
static const UByte SB_DSP_WRITE = 0xc;
static const UByte SB_DSP_WRITESTATUS = SB_DSP_WRITE;
static const UByte SB_DSP_INTR1 = 0xe;
static const UByte SB_DSP_READSTATUS = SB_DSP_INTR1;
static const UByte SB_DSP_INTR2 = 0xf;
static const UByte SB_MIXER_CONTROL = 0x4;
static const UByte SB_MIXER_DATA = 0x5;

// The real address of our DMA buffer
static const L4_Word_t DMABufferRealAddress = 0x20000;

UShort baseIO IS_PERSISTENT;
AudioParameters IS_PERSISTENT params;
L4_ThreadId_t IS_PERSISTENT owner;

/**
 * This class directly interacts with the SB16 mixer
 * to query/set up mixer settings.
 */
class SB16Mixer
{
public:
    
private:
    UShort baseIO;
    UShort control_port() { return baseIO + SB_MIXER_CONTROL; }
    UShort data_port() { return baseIO + SB_MIXER_DATA; }
    
public:
    void init(UShort port);
    
    UByte queryVolume(AudioChannel channel);
    void setVolume(AudioChannel channel, UByte volume);
};

void
SB16Mixer::init(UShort port)
{
    baseIO = port;
}

UByte
SB16Mixer::queryVolume(AudioChannel channel)
{
    outb(control_port(), channel);
    return inb(data_port()) >> 3;
}

void
SB16Mixer::setVolume(AudioChannel channel, UByte volume)
{
    // Volume ranges from 0..15
    if (volume >= 0x20) volume = 0x1f;
    outb(control_port(), channel);
    outb(data_port(), volume << 3);
}

static L4_Time_t WAIT_DELAY;

// The buffer from which the DMA will read our data
UByte*  DMABuffer;

// The buffer to which the client will write audio data
UByte*  DMABufferClient;

class SB16Driver : public DeviceDriver
{
private:
    SB16Mixer mixer;
    UShort sb_port(UByte port) { return baseIO + port; }
   
    static const UInt bufSize = 0x1000;
    
    void dsp_write(UByte value);
    UByte dsp_read();
    stat_t initialize_dsp(UShort baseport);
    stat_t set_sampling_rate(UShort rate);
    
    stat_t setup(const AudioParameters newParams);
    
    bool isOwnedBy(const L4_ThreadId_t &tid)
        { return owner == tid; }
    
    void play();
    void stop();
    
public:
    SB16Driver();
    const char *const Name() { return "sb16"; }
    
    stat_t Initialize();
    stat_t Recover();
    stat_t Exit();
    
    stat_t Service(const L4_ThreadId_t &tid, L4_Msg_t& msg);
    
    stat_t HandleInit(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandlePlay(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandleStop(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandleSetVolume(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandleGetVolume(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandleShutdown(const L4_ThreadId_t &tid, L4_Msg_t *msg);
    stat_t HandleBufferFilled(const L4_ThreadId_t &tid, L4_Msg_t *msg);
};

SB16Driver::SB16Driver()
{
   owner = L4_nilthread;
}

void
SB16Driver::dsp_write(UByte value)
{
    while ((inb(sb_port(SB_DSP_WRITESTATUS)) & 0x80) != 0);
    outb(sb_port(SB_DSP_WRITE), value);
}

UByte
SB16Driver::dsp_read()
{
    while ((inb(sb_port(SB_DSP_READSTATUS)) & 0x80) == 0);
    return inb(sb_port(SB_DSP_READ));
}

stat_t
SB16Driver::initialize_dsp(UShort basePort)
{
    const L4_Time_t pool_delay = L4_TimePeriod(1);
    // We leave at least 200 microseconds for the DSP to initialize - this 
    // is more than enough.
    const UInt  max_pooling = 200;  
    UInt    pooling_count = 0;
    UByte   status;

    baseIO = basePort;

    // Reset the DSP
    outb(sb_port(SB_DSP_RESET), 1);
    L4_Sleep(WAIT_DELAY);
    outb(sb_port(SB_DSP_RESET), 0);

    status = 0;
    while (!(status & 0x80)) {
        status = inb(sb_port(SB_DSP_INTR1));
        L4_Sleep(pool_delay);
        if (pooling_count++ == max_pooling) {
            goto init_failure;
        }
    }
    status = 0;
    while (status != 0xaa) {
        status = inb(sb_port(SB_DSP_READ));
        L4_Sleep(pool_delay);
        if (pooling_count++ == max_pooling) {
            goto init_failure;
        }
    }
    return ERR_NONE;
    
init_failure:
    return ERR_NOT_FOUND;
}

stat_t
SB16Driver::setup(const AudioParameters newParams)
{
    params = newParams;
    
    // Now initialize the DMA buffer - we allocate a whole page for it, which
    // we will take from the first megabyte of memory. As it is reserved by
    // the page manager, no other process should use it - still, this way of
    // doing is not clean at all! We need a dedicated memory manager to handle
    // the first megabyte of memory as IO memory.
    // First reserve area in the address space - we will also waste a physical
    // memory page in the process.
    DMABufferClient = (UByte *)palloc(1);
    // Workaround to make sure a physical page is mapped - we need to access the virtual address
    memset(DMABufferClient, 0, PAGE_SIZE);
    DMABuffer = (UByte *)palloc(1);
    
    // XXX: Directly ask the page from Sigma0 - this should be punished by
    // death, but for now this is a quick'n dirty way to get what we need.
    // We chose page at address 0x20000 to be our DMA buffer
    stat_t status = Pager.Map(reinterpret_cast<addr_t>(DMABuffer),
                              L4_ReadWriteOnly, DMABufferRealAddress,
                              L4_nilthread /* Direct mapping */);
    //status = MapSigma0Page(DMABufferRealAddress, (L4_Word_t)DMABuffer);
    if (status != ERR_NONE) {
        System.Print(System.ERROR, "sb16: Cannot map DMA buffer!\n");
        return status;
    }

    // Prepare the DMA transfer
    status = dma_prepare_transfer(params.is16Bits ? SB_DMA2 : SB_DMA1, (DMATransferMode) (SINGLE | ADRINC), FROMMEMORY, DMABufferRealAddress, PAGE_SIZE);
    
    if (status != ERR_NONE) {
        System.Print(System.ERROR, "sb16: Cannot set up DMA transfer\n");
        return status;
    }
    
    return ERR_NONE;
}

stat_t
SB16Driver::set_sampling_rate(UShort srate)
{
    // Issue command 0x41, then write the high and low bits
    // of the sampling rate
    outb(sb_port(SB_DSP_WRITE), 0x41);
    outb(sb_port(SB_DSP_WRITE), (srate >> 8) & 0xff);
    outb(sb_port(SB_DSP_WRITE), srate & 0xff);
    return ERR_NONE;
}

void
SB16Driver::play()
{
    // Program the sound card
    // Setup the sampling rate
    set_sampling_rate(params.rate);
    // Write I/O command (16 bits samples)
    if (params.is16Bits) dsp_write(0xb6);
    else dsp_write(0xc6);
    // Write I/O transfer mode
    dsp_write((params.isSigned << 4) | (params.isStereo << 5));
    // Write block size - here it is half of a page
    // Low byte
    dsp_write((bufSize / 2) & 0xff);
    // High byte
    dsp_write(((bufSize / 2) >> 8) & 0xff);
}

void SB16Driver::stop()
{
    if (params.is16Bits) dsp_write(0xd5);
    else dsp_write(0xd0);
}

stat_t
SB16Driver::Initialize()
{
    System.Print(System.INFO, "SB16 driver started\n");
    WAIT_DELAY = L4_TimePeriod(3);
    stat_t status = initialize_dsp(SB_BASE_IO);
    mixer.init(SB_BASE_IO);
    if (status != ERR_NONE) {
        System.Print(System.ERROR, "Cannot initialize SB16 DSP at port 0x%x\n",
                     SB_BASE_IO);
        return status;
    }
    dsp_write(0xe1);
    UByte major = dsp_read();
    UByte minor = dsp_read();
    if (major < 4) {
        System.Print(System.ERROR,
                 "DSP version %d detected - this driver requires 4 or more\n",
                 major);
        return ERR_NOT_FOUND;
    }
    System.Print(System.INFO,
                 "DSP Initialized, port 0x%x, DSP version %d.%d\n",
                 SB_BASE_IO, major, minor);
    
    mixer.setVolume(MASTERLEFT, 15);
    mixer.setVolume(MASTERRIGHT, 15);
    mixer.setVolume(VOCLEFT, 15);
    mixer.setVolume(VOCRIGHT, 15);

    System.Print("Volumes:\n");
    System.Print("Master left: %d\n",
                 mixer.queryVolume(MASTERLEFT));
    System.Print("Master right: %d\n",
                 mixer.queryVolume(MASTERRIGHT));
    System.Print("Voc left: %d\n", mixer.queryVolume(VOCLEFT));
    System.Print("Voc right: %d\n", mixer.queryVolume(VOCRIGHT));
    
    
    
    
    System.Print("OKI\n");

    return ERR_NONE;
}

stat_t
SB16Driver::Recover() {
    System.Print("recovery parameters: baseIO: 0x%x, rate: %d, owner: %x\n", baseIO, params.rate, owner.raw);
    return ERR_NONE;
}

stat_t
SB16Driver::Exit() {
    return ERR_NONE;
    
}

stat_t SB16Driver::HandleInit(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // First check the driver is not taken by another program 
    if (!(owner == L4_nilthread || owner == tid)) return ERR_BUSY;
    owner = tid;
    
    // Get the parameters
    AudioParameters newParams;
    newParams.raw = L4_Get(msg, 0);
    setup(newParams);
    // Now map the audio buffer page to the requesting process and send the sound card IRQ
    L4_MapItem_t bufferMap = L4_MapItem(L4_FpageAddRights(L4_Fpage(reinterpret_cast<L4_Word_t>(DMABufferClient), PAGE_SIZE), L4_FullyAccessible), reinterpret_cast<L4_Word_t>(DMABufferClient));
    L4_Word_t cardIrq = SB_IRQ;
    L4_Put(msg, 0, 1, &cardIrq, 2, &bufferMap);
    return ERR_NONE;
}

stat_t SB16Driver::HandlePlay(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    play();
    return ERR_NONE;
}

stat_t SB16Driver::HandleStop(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    stop();
    return ERR_NONE;
}

stat_t SB16Driver::HandleSetVolume(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    AudioChannel channel = static_cast<AudioChannel>(L4_Get(msg, 0));
    L4_Word_t volume = L4_Get(msg, 1);
    
    mixer.setVolume(channel, volume);
    
    return ERR_NONE;
}

stat_t SB16Driver::HandleGetVolume(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    AudioChannel channel = static_cast<AudioChannel>(L4_Get(msg, 0));
    
    L4_Word_t volume = mixer.queryVolume(channel);
    L4_Put(msg, 0, 1, &volume, 0, 0);
    
    return ERR_NONE;
}

stat_t SB16Driver::HandleShutdown(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    stop();
    owner = L4_nilthread;
    return ERR_NONE;
}

stat_t SB16Driver::HandleBufferFilled(const L4_ThreadId_t &tid, L4_Msg_t *msg)
{
    // Check that the requesting thread owns the audio device
    // Actually, don't - because the interrupt handler will call us.
    // if (!isOwnedBy(tid)) return ERR_NOT_FOUND;
    
    // Copy the content of the client buffer into the DMA buffer
    DOUT("%p %p\n", DMABuffer, DMABufferClient);
    //for (UInt i = 0; i < PAGE_SIZE; i++) System.Print("%d ", DMABuffer[i]);
    //System.Print("\n");
    //for (int i = 0; i < PAGE_SIZE; i++) DMABuffer[i] = DMABufferClient[i];
    memcpy(DMABuffer, DMABufferClient, PAGE_SIZE);
    
    // Acknowledge the card that the buffer is filled
    // Upon 16 bits
    if (params.is16Bits) inb(SB_BASE_IO + SB_DSP_INTR2);
    // Upon 8 bits
    else inb(SB_BASE_IO + SB_DSP_INTR1);
    // Acknowledge end of interrupt to the PIC
    outb(0x20, 0x20);
    return ERR_NONE;
}

ARC_DRIVER(SB16Driver);

BEGIN_HANDLERS(SB16Driver)
    CONNECT_HANDLER(MSG_AUDIO_INIT, HandleInit)
    CONNECT_HANDLER(MSG_AUDIO_PLAY, HandlePlay)
    CONNECT_HANDLER(MSG_AUDIO_STOP, HandleStop)
    CONNECT_HANDLER(MSG_AUDIO_GETVOLUME, HandleGetVolume)
    CONNECT_HANDLER(MSG_AUDIO_SETVOLUME, HandleSetVolume)
    CONNECT_HANDLER(MSG_AUDIO_SHUTDOWN, HandleShutdown)
    CONNECT_HANDLER(MSG_AUDIO_BUFFERFILLED, HandleBufferFilled)
END_HANDLERS
