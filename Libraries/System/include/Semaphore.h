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
/// @brief  Semaphore
/// @file   Include/arc/semaphore.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Semaphore.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_SEMAPHORE_H
#define ARC_SEMAPHORE_H

#include <l4/thread.h>
#include <Mutex.h>
#include <Types.h>

class Semaphore
{
private:
    Mutex           _mutex;
    volatile int    _sem;
    int             _max;

public:
    Semaphore(size_t n) : _max(n) {};

    void Up();
    void Down();
};

inline void
Semaphore::Up()
{
begin:
    while (_max != 0 && _sem == _max) {
        L4_ThreadSwitch(L4_nilthread);
    }
    _mutex.Lock();
    if (_max == 0 || _sem < _max) {
        _sem++;
    }
    else {
        _mutex.Unlock();
        goto begin;
    }
    _mutex.Unlock();
}

inline void
Semaphore::Down()
{
begin:
    while (_sem == 0) {
        L4_ThreadSwitch(L4_nilthread);
    }
    _mutex.Lock();
    if (_sem > 0) {
        _sem--;
    }
    else {
        _mutex.Unlock();
        goto begin;
    }
    _mutex.Unlock();
}

#endif // ARC_SEMAPHORE_H
