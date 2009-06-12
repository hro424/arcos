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
/// @file   Libraries/Level2/include/Console.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Console.h 384 2008-08-28 18:38:10Z hro $

#ifndef ARC_MICRO_SHELL_CONSOLE_H
#define ARC_MICRO_SHELL_CONSOLE_H

#include <Mutex.h>
#include <Thread.h>
#include <Types.h>
#include <Stream.h>
#include <Session.h>

class LineBuffer;

class ConsoleReader : public Thread<>
{
private:
    L4_ThreadId_t   _input;
    LineBuffer&     _buffer;
    Mutex           _mutex;
    volatile Bool   _running;

public:
    ConsoleReader(LineBuffer& buf);
    virtual ~ConsoleReader();
    virtual void Run();
    Bool IsActive();
};


class ConsoleWriter
{
protected:
    ///
    /// Session with the console server
    ///
    Session*    _session;

    ///
    /// The beginning of the shared memory area
    ///
    addr_t      _buffer_head;

    ///
    /// The end of the shared memory area
    ///
    addr_t      _buffer_tail;

    ///
    /// The beginning of the current window
    ///
    addr_t      _head;

    ///
    /// The end of the current window
    ///
    addr_t      _tail;
public:

    ///
    /// Initializes a ConsoleWrite object.  Establishes a shared memory area
    /// with a cosole server.
    ///
    ConsoleWriter();

    ///
    /// Flush the current buffer then, destroys the shared memory.
    ///
    virtual ~ConsoleWriter();

    ///
    /// Writes the data to the buffer.  Makes a transfer if the buffer
    /// becomes full or '\n's are contained in the data.
    ///
    virtual stat_t Write(const char* buf, size_t len, size_t* wsize);

    ///
    /// Transfers the current data in the buffer to the console server.
    ///
    virtual stat_t Flush();
};


class Console : public Stream
{
private:
    Mutex            _read_lock;
    Mutex            _write_lock;
    LineBuffer*      _input_buffer;
    ConsoleReader*   _reader;
    ConsoleWriter*   _writer;

    stat_t InitializeInput();
    stat_t InitializeOutput();
    void Reader();
    void Writer();

public:
    static const char*      INPUT_SERVER;
    static const char*      OUTPUT_SERVER;

    ///
    /// Initializes the Console object.
    ///
    /// @param bufsize      the buffer size of the input
    ///
    void Initialize(size_t bufsize);
    void Stop();

    void Lock() {}
    void Unlock() {}

    ///
    /// Reads a single character from the console.
    ///
    Int Read();

    ///
    /// Reads user input.
    ///
    /// @param buf          the buffer to hold the user input
    /// @param len          the length of the buffer
    /// @param rlen         the length of input characters
    ///
    stat_t Read(void* buf, size_t len, size_t* rlen = 0);

    ///
    /// Writes a single character to the console.
    ///
    void Write(Int c);

    ///
    /// Writes data to the console.
    ///
    /// @param buf          the buffer holding the data to be output
    /// @param len          the length of the buffer
    /// @param wlen         the length of actual written data
    ///
    stat_t Write(const void* buf, size_t len, size_t* wlen = 0);

    void Flush();

    char* ReadLine();

    void WriteLine(const char* str);

    void Put(char c);

    char Get();
};

#endif // ARC_MICRO_SHELL_CONSOLE_H

