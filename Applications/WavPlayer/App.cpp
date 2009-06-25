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
/// @brief  A wav file player
/// @file   Applications/WavPlayer/App.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  September 2008
///

//$Id: App.cpp 413 2008-09-09 01:38:42Z hro $

#include <Debug.h>
#include <audio/Audio.h>
#include <FileStream.h>
#include <Ipc.h>
#include <NameService.h>
#include <System.h>
#include <Types.h>

#define ASYNC

#ifdef ASYNC

// The size of the buffers
#define BUFFER_SIZE     PAGE_SIZE

// Buffer No.1
static UByte            _source_1[BUFFER_SIZE];

// Buffer No.2
static UByte            _source_2[BUFFER_SIZE];

// Pointer to the _current buffer
static UByte*           _current_source;

// Pointer to the next buffer
static UByte*           _next_source;

// Pointer to the previous buffer
static UByte*           _prev_source;

// Cursor in the buffer
static UByte*           _current;

#endif // ASYNC

// Mutex for switching the buffer pointers
static Mutex            _mutex;

// Main thread
static L4_ThreadId_t    _main_thread;

static FileStream       _fstream;


#ifdef ASYNC

///
/// Requests to fill the next buffer
///
static void
request_filling()
{
    L4_Msg_t    msg;
    Ipc::Send(_main_thread, &msg);
}

static stat_t
fill_buffer()
{
    if (_prev_source != 0) {
        // Fill the buffer
        size_t rsize;
        stat_t err = _fstream.Read(_prev_source, BUFFER_SIZE, &rsize);

        if (rsize == 0) {
            err = ERR_OUT_OF_RANGE;
        }

        if (err != ERR_NONE) {
            DOUT("err %u rsize %u\n", err, rsize);
            return err;
        }

        _mutex.Lock();
        _next_source = _prev_source;
        _mutex.Unlock();
    }
    return ERR_NONE;
}

#endif // ASYNC

///
/// Call back function invoked by the Audio driver.
///
void
callback(size_t size, UByte* buffer)
{
    DOUT("SB Interrupt, size: %d\n", size);
    for (size_t i = 0; i < size; i++) {
#ifdef ASYNC
        // Copy the _current buffer
        buffer[i] = *_current;
        _current++;
        if (_current_source + BUFFER_SIZE < _current) {
            // Change to the next buffer
            _mutex.Lock();
            _prev_source = _current_source;
            _current_source = _next_source;
            _mutex.Unlock();

            _current = _current_source;
            request_filling();
        }
#else // ASYNC
        //FIXME: Synchronous mode doesn't work because the context of this
        //      callback thread is different from that of the main one.  This
        //      means that the file system doesn't recognize the callback
        //      thread.  Consequently, FileStream.Read receives a file-not-
        //      found.
        size_t  rsize;
        stat_t  err;

        DOUT("fill the buffer\n");
        err = _fstream.Read(buffer, size, &rsize);
        if (rsize == 0 || err != ERR_NONE) {
            DOUT("err %u rsize %u\n", err, rsize);
            L4_Msg_t    dummy;
            Ipc::Send(_main_thread, &dummy);
        }
#endif // ASYNC
    }
}

static void
play()
{
    ENTER;
    DOUT("play\n");
    stat_t err = Audio::Play();
    if (err != ERR_NONE) {
        DOUT("Error %u\n", err);
    }

#ifdef ASYNC
    for (;;) {
        L4_Receive(Audio::CallbackId());
        if (fill_buffer() != ERR_NONE) {
            break;
        }
    }
#endif

    Audio::Stop();
    EXIT;
}

static stat_t
init_file(const char* server, const char* path)
{
    stat_t          err;
    L4_ThreadId_t   fs_id;

    DOUT("look up '%s'\n", server);

    err = NameService::Get(server, &fs_id);
    if (err != ERR_NONE) {
        return err;
    }

    DOUT("fs id %.8lX\n", fs_id.raw);

    err = _fstream.Connect(fs_id);
    if (err != ERR_NONE) {
        return err;
    }

    DOUT("open %s\n", path);

    err = _fstream.Open(path, FileStream::READ);
    if (err != ERR_NONE) {
        return err;
    }

    DOUT("size %d\n", _fstream.Size());

    return ERR_NONE;
}


static stat_t
init_audio(const char* audio_server)
{
    AudioParameters params;
    stat_t          err;

#ifdef ASYNC
    size_t          rsize;

    _current_source = _source_1;
    _next_source = _source_2;

    // Fill the initial buffer
    err = _fstream.Read(_source_1, BUFFER_SIZE, &rsize);
    if (rsize == 0) {
        err = ERR_OUT_OF_RANGE;
    }

    if (err != ERR_NONE) {
        return err;
    }

    err = _fstream.Read(_source_2, BUFFER_SIZE, &rsize);
    if (rsize == 0) {
        err = ERR_OUT_OF_RANGE;
    }

    if (err != ERR_NONE) {
        return err;
    }

    _current = _current_source;
#endif // ASYNC

    DOUT("Audio setup\n");
    params.rate = 22050;
    params.isSigned = false;
    params.is16Bits = false;
    params.isStereo = false;
    // NOTE: The callback function is called once by the initialization of
    // the Audio driver.
    err = Audio::Initialize(audio_server, params, callback);

    return err;
}

static void
fini()
{
    DOUT("audio shutdown...\n");
    Audio::Shutdown();
    DOUT("audio shutdown done\n");

    _fstream.Close();
    DOUT("fs closed\n");

    _fstream.Disconnect();
    DOUT("fs disconnected\n");
}

int
main(int argc, char* argv[])
{
    if (argc < 3) {
        System.Print("usage: player audio_server file_server file\n");
        return 0;
    }

    if (init_file(argv[2], argv[3]) != ERR_NONE) {
        return -1;
    }

    if (init_audio(argv[1]) != ERR_NONE) {
        return -1;
    }

    _main_thread = L4_Myself();

    play();
    DOUT("end of playback\n");

    fini();

    return 0;
}

