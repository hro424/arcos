/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
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
/// @file   Libraries/System/Debug.cpp
/// @brief  Debugging helper
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: Debug.cpp 353 2008-05-30 04:40:04Z hro $

#include <Debug.h>
#include <DebugStream.h>
#include <Format.h>
#include <arc/stdarg.h>
#include <l4/kdebug.h>
#include <l4/thread.h>

#ifndef SYS_DEBUG_LEVEL
#define SYS_DEBUG_LEVEL 0
#endif

DebugHelper Debug(SYS_DEBUG_LEVEL, &__dstream);

DebugHelper::DebugHelper(UInt level, Stream* s)
    : _level(level), _stream(s)
{
}

void
DebugHelper::Print(UInt level, const char* fmt, ...)
{
    if (level > _level) {
        va_list ap;
        va_start(ap, fmt);
        Formatter::Write(_stream, -1, fmt, ap);
        va_end(ap);
    }
}

void
DebugHelper::Print(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Formatter::Write(_stream, -1, fmt, ap);
    va_end(ap);
}

void
DebugHelper::Break(const char* msg)
{
    Print(msg);
    L4_KDB_Enter("break");
}

void
DebugHelper::Break(const char* msg, const char* file, UInt line)
{
    Print("%s: %d: break %s\n", file, line, msg);
    L4_KDB_Enter("break");
}


void
__assert(const char *expr, const char *file, int line)
{
    Debug.Print("[%.8lX] %s: %d: assertion (%s) failed\n",
                L4_Myself().raw, file, line, expr);
    L4_KDB_Enter("assert");
}

void
__enter(const char* const file, int line, const char* const func)
{
    //Debug.Print("\e[32m%.8lX %s: %d Enter %s\n\e[0m",
    //            L4_Myself().raw, file, line, func);
    Debug.Print("\e[32m[%.8lX] Enter %s\n\e[0m", L4_Myself().raw, func);
}

void
__exit(const char* const file, int line, const char* const func)
{
    //Debug.Print("\e[32m%.8lX %s: %d Exit %s\n\e[0m",
    //            L4_Myself().raw, file, line, func);
    Debug.Print("\e[32m[%.8lX] Exit %s\n\e[0m", L4_Myself().raw, func);
}

