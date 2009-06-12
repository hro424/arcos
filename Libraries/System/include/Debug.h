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

//$Id: Debug.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_DEBUG_H
#define ARC_DEBUG_H

#include <Types.h>

class Stream;

class DebugHelper
{
private:
    UInt    _level;
    Stream* _stream;

    DebugHelper() {}

public:
    DebugHelper(UInt level, Stream* s);
    void Print(UInt level, const char* fmt, ...);
    void Print(const char* fmt, ...);
    void Break(const char* msg);
    void Break(const char* msg, const char* file, UInt ln);
};

extern DebugHelper Debug;

#ifdef SYS_DEBUG
#include <l4/thread.h>

#define BREAK(msg)          Debug.Break(#msg, __FILE__, __LINE__)

#define DOUT(fmt, args...)      \
    Debug.Print("\e[1m\e[32m[%.8lX] %s %d: " fmt "\e[0m",    \
                L4_Myself().raw, __func__,              \
                __LINE__, ##args)

#  ifdef SYS_DEBUG_CALL

void __enter(const char* const file, int ln, const char* const func);
void __exit(const char* const file, int ln, const char* const func);

#define ENTER           __enter(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#define EXIT            __exit(__FILE__, __LINE__, __PRETTY_FUNCTION__)

#  else // !SYS_DEBUG_CALL

#define ENTER
#define EXIT

#  endif //SYS_DEBUG_CALL

#else // !SYS_DEBUG

#define BREAK(msg)
#define DOUT(fmt, args...)
#define ENTER
#define EXIT

#endif // SYS_DEBUG


#endif // ARC_DEBUG_H

