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
/// @brief  A wrapper class for the console
/// @file   Librareis/Level2/src/Console.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Console.cc 386 2008-08-30 05:07:57Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <NameService.h>
#include <Session.h>
#include <Thread.h>
#include <l4/schedule.h>
#include "Console.h"
#include "LineBuffer.h"
#include "Scanner.h"

extern Console __console;

const char* Console::INPUT_SERVER = "kb";
const char* Console::OUTPUT_SERVER = "console";

//---------------------------------------------------------------------------
//  Console Helper classes
//---------------------------------------------------------------------------

ConsoleReader::ConsoleReader(LineBuffer& buf) : _buffer(buf), _running(FALSE)
{
}

ConsoleReader::~ConsoleReader()
{
    _mutex.Lock();
    _running = FALSE;
    Cancel();
    _mutex.Unlock();
}

Bool
ConsoleReader::IsActive()
{
    return _running;
}

void
ConsoleReader::Run()
{
    L4_ThreadId_t   server;
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    stat_t          err;
    Scanner         scanner;

    err = NameService::Get(Console::INPUT_SERVER, &server);
    if (err != ERR_NONE) {
        return;
    }

    L4_Put(&msg, MSG_EVENT_CONNECT, 0, 0, 0, 0);
    err = Ipc::Call(server, &msg, &msg);
    if (err != ERR_NONE) {
        return;
    }

    _input.raw = L4_Get(&msg, 0);
    _running = TRUE;

    for (;;) {
        _mutex.Lock();
        if (_running == FALSE) {
            _mutex.Unlock();
            break;
        }
        else {
            _mutex.Unlock();
        }

        tag = L4_Receive(_input);
        if (L4_Label(tag) == MSG_EVENT_NOTIFY) {

            L4_Store(tag, &msg);

            L4_Word_t code = L4_Get(&msg, 0);
            UInt data = scanner.Scan(code);

            if (0 < data && data < 0x100) {
                char c = static_cast<char>(data);

                // Edit the line buffer
                if (c == '\b') {
                    if (_buffer.Back()) {
                        __console.Put('\b');
                        __console.Put(' ');
                    }
                    else {
                        continue;
                    }
                }
                else if (c == '\r' || c == '\n') {
                    // Pass the buffer to the readline method
                    _buffer.Put('\0');
                    _buffer.Notify();
                }
                else {
                    _buffer.Put(c);
                }

                // Give a feedback
                __console.Put(c);
                __console.Flush();
            }
            else {
                // Modifier key is pressed
            }

        }
        else if (L4_Label(tag) == MSG_EVENT_TERMINATE) {
            return;
        }
    }

    L4_Put(&msg, MSG_EVENT_DISCONNECT, 0, 0, 0, 0);
    Ipc::Call(server, &msg, &msg);
}


ConsoleWriter::ConsoleWriter()
    : _buffer_head(0), _buffer_tail(0), _head(0), _tail(0)
{
    L4_ThreadId_t   server;
    stat_t          err;

    err = NameService::Get(Console::OUTPUT_SERVER, &server);
    if (err != ERR_NONE) {
        return;
    }

    _session = new Session(server);
    if (_session == 0) {
        return;
    }

    if (!_session->IsConnected()) {
        delete _session;
        FATAL("Output not found\n");
        return;
    }

    _buffer_head = _session->GetBaseAddress();
    _buffer_tail = _buffer_head + _session->Size();
    _head = _buffer_head;
    _tail = _buffer_head;
}

ConsoleWriter::~ConsoleWriter()
{
    Flush();
    delete _session;
}

stat_t
ConsoleWriter::Write(const char* buf, size_t count, size_t* wsize)
{
    for (size_t i = 0; i < count; i++) {
        *reinterpret_cast<char*>(_tail) = buf[i];
        _tail++;
        if (buf[i] == '\n' || _tail == _head) {
            Flush();
        }
        if (_tail == _buffer_tail) {
            _tail = _buffer_head;
        }
    }

    return ERR_NONE;
}

stat_t
ConsoleWriter::Flush()
{
    stat_t      err = ERR_NONE;
    L4_Word_t   reg[2];

    reg[0] = _head - _session->GetBaseAddress();
    reg[1] = _tail - _session->GetBaseAddress();
    err = _session->Put(reg, 2);
    _head = _tail;

    return err;
}


//---------------------------------------------------------------------------
//  Console implemenation
//---------------------------------------------------------------------------


// Must finish before 'main'
void
Console::Initialize(size_t size)
{
    _input_buffer = new LineBuffer(size);

    // Create the reader and the writer threads
    _reader = new ConsoleReader(*_input_buffer);
    _writer = new ConsoleWriter();

    _reader->Start();
}

void
Console::Stop()
{
    delete _writer;
    delete _reader;
    delete _input_buffer;
}

stat_t
Console::Read(void* buf, size_t len, size_t *rlen)
{
    ScopedLock  lock(&_read_lock);
    size_t      size;

    size = _input_buffer->Read(static_cast<char*>(buf), len);
    if (rlen != 0) {
        *rlen = size;
    }
    return ERR_NONE;
}

stat_t
Console::Write(const void* buf, size_t len, size_t *wlen)
{
    ScopedLock  lock(&_write_lock);

    return _writer->Write(static_cast<const char*>(buf), len, wlen);
}

void
Console::Flush()
{
    ScopedLock lock(&_write_lock);
    _writer->Flush();
}

char*
Console::ReadLine()
{
    _input_buffer->Wait();
    return _input_buffer->ReadLine();
}

void
Console::Put(char c)
{
    ScopedLock lock(&_write_lock);
    _writer->Write(&c, 1, 0);
}

void
Console::Write(Int c)
{
    Put(c);
}

char
Console::Get()
{
    char c;
    ScopedLock lock(&_read_lock);
    _input_buffer->Read(&c, 1);
    return c;
}

Int
Console::Read()
{
    return Get();
}

