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
/// @file   Libraries/Level2/include/LineBuffer.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  Feburuary 2008
///

//$Id: LineBuffer.h 375 2008-08-08 07:53:30Z hro $

#ifndef ARC_MICRO_SHELL_LINE_BUFFER_H
#define ARC_MICRO_SHELL_LINE_BUFFER_H

#include <RingBuffer.h>
#include <Semaphore.h>

class LineBuffer : public RingBuffer
{
private:
    Semaphore*  _semaphore;

public:
    LineBuffer(size_t size);
    virtual ~LineBuffer();

    virtual char *ReadLine();
    virtual void Put(char c);
    virtual bool Back();
    virtual void Wait();
    virtual void Notify();
};

LineBuffer::LineBuffer(size_t size) : RingBuffer(size)
{
    // This semaphore has no limit.
    _semaphore = new Semaphore(0);
}

LineBuffer::~LineBuffer()
{
    delete _semaphore;
}

inline char*
LineBuffer::ReadLine()
{
    if (_read_ptr == _write_ptr) {
        return 0;
    }

    char*   ptr = _read_ptr;
    while (*_read_ptr != '\0') {
        _read_ptr++;
        _count--;
    }
    _read_ptr++;
    _count--;
    return ptr;
}

inline void
LineBuffer::Put(char c)
{
    Write(&c, 1);
}

inline bool
LineBuffer::Back()
{
    if (_count == 0) {
        return false;
    }

    if (_write_ptr == _head) {
        _write_ptr = _tail;
    }
    _write_ptr--;
    *_write_ptr = 0;
    _count--;
    return true;
}

inline void
LineBuffer::Wait()
{
    _semaphore->Down();
}

inline void
LineBuffer::Notify()
{
    _semaphore->Up();
}

#endif // ARC_MICRO_SHELL_LINE_BUFFER_H

