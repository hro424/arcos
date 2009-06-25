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
/// @brief  A ring buffer
/// @file   Libraries/Arc/ringbuf.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: RingBuffer.cpp 349 2008-05-29 01:54:02Z hro $

#include <MemoryAllocator.h>
#include <RingBuffer.h>
#include <System.h>

RingBuffer::RingBuffer(size_t size = DEFAULT_BUFFER_SIZE)
    : _size(size), _count(0)
{
    _head = static_cast<char*>(malloc(size));
    if (_head == 0) {
        FATAL("RingBuffer: out of memory");
    }

    _read_ptr = _write_ptr = _head;
    _tail = _head + _size;
}

size_t
RingBuffer::Read(char* buf, size_t len)
{
    if (buf == 0 || len == 0 || _count == 0) {
        return 0;
    }

    size_t  count = (len < _count) ? len : _count;
    char*   ptr = buf;

    for (UInt i = 0; i < count; i++) {
        if (_read_ptr == _tail) {
            _read_ptr = _head;
        }
        *ptr = *_read_ptr;
        ptr++;
        _read_ptr++;
        _count--;
    }

    return count;
}

size_t
RingBuffer::Write(const char* buf, size_t len)
{
    if (buf == 0 || len == 0 || _count == _size) {
        return 0;
    }

    size_t count = _size - _count;
    if (len < count) {
        count = len;
    }

    const char* ptr = buf;
    for (UInt i = 0; i < count; i++) {
        if (_write_ptr == _tail) {
            _write_ptr = _head;
        }
        *_write_ptr = *ptr;
        ptr++;
        _write_ptr++;
        _count++;
    }

    return count;
}

