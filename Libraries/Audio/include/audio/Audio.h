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

//$Id: Audio.h 410 2008-09-08 12:00:15Z hro $

#include <Thread.h>
#include <Interrupt.h>

#ifndef CLIENT_AUDIO_H_
#define CLIENT_AUDIO_H_

typedef union {
    L4_Word_t raw;
    
    struct {
        L4_Word_t rate : sizeof(UShort) * 8;
        L4_Word_t isSigned : 1;
    	L4_Word_t is16Bits : 1;
    	L4_Word_t isStereo : 1;
    };
} AudioParameters;

typedef enum {
    MASTERLEFT = 0x30,
    MASTERRIGHT = 0x31,
    VOCLEFT = 0x32,
    VOCRIGHT = 0x33,
} AudioChannel;

typedef void (*sndint_callback) (size_t size, UByte * buffer);

class SB16IntHandler : public InterruptHandler
{
private:
    sndint_callback callback;

public:
    SB16IntHandler(sndint_callback cback);
    virtual ~SB16IntHandler();
    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg);
    void HandleInterrupt(); 
};
    
class Audio
{
private:
    static const char* const    SERVER_NAME;
    static L4_ThreadId_t        _server;
    static SB16IntHandler*      _handler;
    static AudioParameters      _params;
    static UByte *              _audioBuffer;
    static UInt                 _bufferPos;
    static UByte                _irqNbr;

    static bool isConnected() { return _server.raw != 0; }
    
public:
    static stat_t Initialize(const char* server_name, AudioParameters params,
                             sndint_callback callback);
    static stat_t Play();
    static stat_t Stop();
    static stat_t SetVolume(const AudioChannel channel, const UByte volume);
    static stat_t GetVolume(const AudioChannel channel, UByte &volume);
    static stat_t Shutdown();
    static L4_ThreadId_t CallbackId();
    friend class SB16IntHandler;
};

#endif /*CLIENT_AUDIO_H_*/
