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
/// @file   Include/arc/System.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  April 2008
///

//$Id: System.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_SYSTEM_H
#define ARC_SYSTEM_H

#include <Types.h>
#include <sys/Config.h>

class Stream;

class SystemHelper
{
public:
    static const Int    INFO = 3;
    static const Int    WARN = 2;
    static const Int    ERROR = 1;

private:
    Int         _level;
    Stream*     _stream;

    SystemHelper();

public:
    SystemHelper(Int level, Stream* s);
    stat_t Read(void* buf, size_t count, size_t* rlen = 0);
    stat_t Write(const void* buf, size_t count, size_t* wlen = 0);
    void Flush();
    char* ReadLine();
    void Print(const char* fmt, ...);
    void Print(Int lv, const char* fmt, ...);
    void Exit(int status);
    void Fatal(const char* msg);
    void Fatal(const char* msg, const char* file, int ln);
    void Assert(const char* expr, const char* file, int ln);
};

extern SystemHelper System;

#define FATAL(msg)      System.Fatal(#msg, __FILE__, __LINE__)
#define SECTION(x)      __attribute__((section(x)))
#define SEC_INIT        ".init"

int atoi(const char* str);

#endif // ARC_SYSTEM_H

