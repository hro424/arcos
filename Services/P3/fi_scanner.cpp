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
/// @brief  IA32 instruction interpreter
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///

//$Id: fi_scanner.cpp 382 2008-08-28 06:48:59Z hro $

#include <Types.h>
#include <String.h>
#include "fi_scanner.h"

scanner::scanner()
    : _start(0), _end(0), _cur(0), _length(0)
{
}

void
scanner::initialize(char* start, char* end)
{
    _start = start;
    _end = end;
    _cur = _start;
    _length = _end - _start;
}

int
scanner::get(unsigned char* c)
{
    if (_cur < _end) {
        *c = *reinterpret_cast<unsigned char*>(_cur);
        _cur++;
        return _cur - 1 - _start;
    }
    else {
        return -1;
    }
}

int
scanner::put(unsigned char c)
{
    if (_cur < _end) {
        *_cur = c;
        _cur++;
        return 0;
    }
    else {
        return -1;
    }
}

size_t
scanner::read(char* buf, size_t count)
{
    if (_end - _cur < static_cast<int>(count)) {
        count = _end - _cur;
    }
    memcpy(buf, _cur, count);
    _cur += count;
    return count;
}

size_t
scanner::write(const char* buf, size_t count)
{
    if (_end - _cur < static_cast<int>(count)) {
        count = _end - _cur;
    }
    memcpy(_cur, buf, count);
    _cur += count;
    return count;
}

int
scanner::seek(int offset, int mode)
{
    int ret = -1;

    switch (mode) {
        case SEEK_CUR:
        {
            char* pos = _cur + offset;
            if (_start <= pos && pos <= _end) {
                _cur = pos;
                ret = _cur - _start;
            }
            // else OVERFLOW
            break;
        }
        case SEEK_SET:
        {
            if (static_cast<int>(_length) < offset) {
                offset = _length;
            }
            _cur = _start + offset;
            ret = _cur - _start;
            break;
        }
        case SEEK_END:
            // OVERFLOW
        default:
            break;
    }
    return ret;
}

