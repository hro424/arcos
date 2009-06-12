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
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// 

//$Id$

#include <NameService.h>
#include <audio/Audio.h>
#include <audio/protocol.h>
#include <System.h>
#include <arc/IO.h>

const char* const Audio::SERVER_NAME = "audio-buffered";
L4_ThreadId_t Audio::_server = { 0 };
UByte * Audio::_audioBuffer;
UInt Audio::_bufferPos;
UByte Audio::_irqNbr;
AudioParameters Audio::_params;

SB16IntHandler::SB16IntHandler(sndint_callback cback) :
    callback(cback)
{
}

SB16IntHandler::~SB16IntHandler()
{
}

void
SB16IntHandler::HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg)
{
    HandleInterrupt();
}

void
SB16IntHandler::HandleInterrupt()
{
    // First write the needed data onto the buffer
    callback(PAGE_SIZE / 2, &Audio::_audioBuffer[Audio::_bufferPos]);
    // Now update the buffer position with the number of bytes that have been
    // written
    Audio::_bufferPos += PAGE_SIZE / 2;
    if (Audio::_bufferPos >= PAGE_SIZE) Audio::_bufferPos = 0;
    
    // Acknowledge the driver that the buffer is filled
    L4_Msg_t msg;
    L4_Put(&msg, MSG_AUDIO_BUFFERFILLED, 0, 0, 0, 0);
    Ipc::Call(Audio::_server, &msg, &msg);    
}

stat_t Audio::Initialize(AudioParameters params, sndint_callback callback) 
{
    UInt bufSize = PAGE_SIZE;
    // Check that the audio buffer size is valid
    
    // Get the ID of the audio server
    stat_t err = NameService::Get(SERVER_NAME, &_server);
    if (err != ERR_NONE) {
        return ERR_NOT_FOUND;
    }
    
    _params = params;
    L4_Msg_t msg;
    L4_Put(&msg, MSG_AUDIO_INIT, 1, &params.raw, 0, 0);
    // Get one page of the address space to receive the sound buffer mapping
    _audioBuffer = (UByte *) palloc(1);
    L4_Accept(L4_MapGrantItems(L4_Fpage(reinterpret_cast<L4_Word_t>(_audioBuffer), PAGE_SIZE)));
    err = Ipc::Call(_server, &msg, &msg);
    L4_Accept(L4_MapGrantItems(L4_Nilpage));
    if (err != ERR_NONE) { return err; }
    // The server gives us a mapping of the audio buffer and the IRQ of
    // the sound card in response
    _irqNbr = static_cast<UByte>(L4_Get(&msg, 0));
    
    // Reset the buffer position
    _bufferPos = 0;
    
    // Now install the  interrupt
    InterruptManager *im = InterruptManager::Instance();
    SB16IntHandler *handler = new SB16IntHandler(callback);
    im->Register(handler, _irqNbr);
    
    // Populate the buffer with initial data
    callback(bufSize, _audioBuffer);
    L4_Put(&msg, MSG_AUDIO_BUFFERFILLED, 0, 0, 0, 0);
    Ipc::Call(_server, &msg, &msg);
    
    return ERR_NONE;
}

stat_t Audio::Play()
{
    if (!isConnected()) return ERR_IPC_NO_PARTNER;
    L4_Msg_t msg;
    L4_Put(&msg, MSG_AUDIO_PLAY, 0, 0, 0, 0);
    stat_t err = Ipc::Call(_server, &msg, &msg);
    if (err != ERR_NONE) { return err; }
    return ERR_NONE;
}

stat_t Audio::Stop()
{
    if (!isConnected()) return ERR_IPC_NO_PARTNER;
    L4_Msg_t msg;
    L4_Put(&msg, MSG_AUDIO_STOP, 0, 0, 0, 0);
    stat_t err = Ipc::Call(_server, &msg, &msg);
    if (err != ERR_NONE) { return err; }
    return ERR_NONE;
}

stat_t Audio::SetVolume(const AudioChannel channel, const UByte volume)
{
    if (!isConnected()) return ERR_IPC_NO_PARTNER;
    L4_Msg_t msg;
    L4_Word_t data[2];
    data[0] = channel;
    data[1] = volume;
    L4_Put(&msg, MSG_AUDIO_SETVOLUME, 2, data, 0, 0);
    stat_t err = Ipc::Call(_server, &msg, &msg);
    if (err != ERR_NONE) { return err; }
    return ERR_NONE;
}

stat_t Audio::GetVolume(const AudioChannel channel, UByte & volume)
{
    if (!isConnected()) return ERR_IPC_NO_PARTNER;
    
    L4_Msg_t msg;
    L4_Word_t data = channel;
    L4_Put(&msg, MSG_AUDIO_GETVOLUME, 1, &data, 0, 0);
    stat_t err = Ipc::Call(_server, &msg, &msg);
    volume = static_cast<UByte>(L4_Get(&msg, 0));
    if (err != ERR_NONE) { return err; }
    return ERR_NONE;
}

stat_t Audio::Shutdown()
{
    if (!isConnected()) return ERR_IPC_NO_PARTNER;
    
    // Uninstall the interrupt
    InterruptManager *im = InterruptManager::Instance();
    im->Deregister(_irqNbr);
    
    L4_Msg_t msg;
    L4_Put(&msg, MSG_AUDIO_SHUTDOWN, 0, 0, 0, 0);
    stat_t err = Ipc::Call(_server, &msg, &msg);
    if (err != ERR_NONE) { return err; }
    return ERR_NONE;
}
