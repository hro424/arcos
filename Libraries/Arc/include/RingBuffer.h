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
/// @file   Include/arc/ringbuf.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: RingBuffer.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_RING_BUFFER_H
#define ARC_RING_BUFFER_H

#include <Types.h>
#include <MemoryAllocator.h>

class RingBuffer
{
protected:
    static const size_t DEFAULT_BUFFER_SIZE = 1024;

    ///
    /// The beginning of the buffer
    ///
    char*   _head;

    ///
    /// The end of the buffer
    ///
    char*   _tail;

    ///
    /// Reader pointer
    ///
    char*   _read_ptr;

    ///
    /// Writer pointer
    ///
    char*   _write_ptr;

    ///
    /// The size of the buffer
    ///
    size_t  _size;

    ///
    /// The size of the data in the buffer
    ///
    size_t  _count;

public:
    RingBuffer(size_t size);
    virtual ~RingBuffer() { mfree(_head); }

    ///
    /// Reads data in the buffer.  The data read is removed from the buffer.
    /// Non-blocking.
    ///
    virtual size_t Read(char* buf, size_t len);

    ///
    /// Write the data into the buffer.
    ///
    virtual size_t Write(const char* buf, size_t len);
};

#endif // ARC_RING_BUFFER_H
