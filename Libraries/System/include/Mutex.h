/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @file   Include/arc/Mutex.h
/// @brief  System/Mutex kit
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Mutex.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_MUTEX_H
#define ARC_MUTEX_H

#include <l4/types.h>
#include <l4/thread.h>

class Mutex
{
private:
    L4_Word_t   _mutex_;

public:
    Mutex() : _mutex_(0) {}

    ~Mutex() {}

    void Initialize() { _mutex_ = 0; }

    int TryLock() {
        L4_Word_t   id = L4_Myself().raw;
        L4_Word_t   ret;

        __asm__ __volatile__ ("lock             \n"
                              "cmpxchgl %1, %2  \n"
                              : "=a" (ret)
                              : "r" (id), "m" (_mutex_), "0" (0)
                              : );
        return ret;
    }

    void Lock() {
        while (TryLock() != 0) {
            L4_ThreadSwitch(L4_nilthread);
        }
    }

    void Unlock() {
        _mutex_ = 0;
    }
};

class ScopedLock
{
private:
    Mutex*      _mutex_;

    ScopedLock();
    ScopedLock(ScopedLock& obj);

public:
    ScopedLock(Mutex* mutex) : _mutex_(mutex) { _mutex_->Lock(); }
    ~ScopedLock() { _mutex_->Unlock(); }
};

#endif  // ARC_MUTEX_H

