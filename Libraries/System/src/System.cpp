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
/// @brief  Primitive functions
/// @file   Libraries/System/System.cpp
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: System.cpp 429 2008-11-01 02:24:02Z hro $

#include <Format.h>
#include <Stream.h>
#include <System.h>
#include <Types.h>
#include <arc/stdarg.h>
#include <sys/Config.h>

#include <l4/kdebug.h>
#include <l4/thread.h>

extern void __exit(int stat);

SystemHelper::SystemHelper() {}

SystemHelper::SystemHelper(Int level, Stream* s)
    : _level(level), _stream(s)
{
}

stat_t
SystemHelper::Read(void* buf, size_t count, size_t* rsize)
{
    return _stream->Read(buf, count, rsize);
}

stat_t
SystemHelper::Write(const void* buf, size_t count, size_t* wsize)
{
    return _stream->Write(buf, count, wsize);
}

void
SystemHelper::Flush()
{
    _stream->Flush();
}

char*
SystemHelper::ReadLine()
{
    return _stream->ReadLine();
}

void
SystemHelper::Print(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Formatter::Write(_stream, -1, fmt, ap);
    va_end(ap);
}

void
SystemHelper::Print(Int lv, const char* fmt, ...)
{
    if (lv <= _level) {
        va_list ap;
        va_start(ap, fmt);
        Formatter::Write(_stream, -1, fmt, ap);
        va_end(ap);
    }
}

void
SystemHelper::Exit(int stat)
{
    __exit(stat);
}

void
SystemHelper::Fatal(const char* msg)
{
    Print("%.8lX '%s'\n", L4_Myself().raw, msg);
    L4_KDB_Enter("fatal");
}

void
SystemHelper::Fatal(const char* msg, const char* file, int ln)
{
    Print("%.8lX '%s' @ %s line:%d\n", L4_Myself().raw, msg, file, ln);
    L4_KDB_Enter("fatal");
}

void
SystemHelper::Assert(const char* expr, const char* file, int ln)
{
    Print("[%.8lX] %s: %d: assertion (%s) failed\n",
          L4_Myself().raw, file, ln, expr);
    L4_KDB_Enter("assert");
}

int
atoi(const char* str)
{
    Bool    minus = FALSE;
    Int     i;
    Int     val;

    // Skip the white space
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    if (*str == '-') {
        minus = TRUE;
        str++;
    }
    else if (*str == '+') {
        str++;
    }

    while (*str == '0') {
        str++;
    }

    i = 0;
    val = 0;
    while (*str != '\0' && i < 10) {
        if ('0' <= *str && *str <= '9') {
            val *= 10;
            val += *str - '0';
            str++;
            i++;
        }
        else {
            return 0;
        }
    }

    if (minus) {
        val *= -1;
    }

    return val;
}

