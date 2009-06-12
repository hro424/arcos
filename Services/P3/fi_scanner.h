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
/// @brief IA32 instruction interpreter
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///

//$Id: fi_scanner.h 382 2008-08-28 06:48:59Z hro $

#ifndef IA32_INTERPRETER_SCANNER_H
#define IA32_INTERPRETER_SCANNER_H

#include <Types.h>

class scanner
{
private:
    char*           _start;
    char*           _end;
    char*           _cur;
    size_t          _length;

public:
    static const int SEEK_CUR = 0;
    static const int SEEK_END = 1;
    static const int SEEK_SET = 2;

    scanner();

    ///
    /// Initializes the scanner.
    ///
    /// @param start    the start address of the text section
    /// @param end      the end address of the text section
    ///
    void initialize(char* start, char* end);

    ///
    /// Obtains one byte at the cursor in the text.
    ///
    int get(unsigned char* c);

    ///
    /// Overwrites one byte at the cursor.
    ///
    int put(unsigned char c);

    ///
    /// Reads multiple bytes from the text.
    ///
    /// @param buf      the buffer
    /// @param count    the size of the buffer
    ///
    size_t read(char* buf, size_t count);

    ///
    /// Overwrites multiple bytes to the text.
    ///
    /// @param buf      the buffer
    /// @param count    the length of the buffer
    ///
    size_t write(const char* buf, size_t count);

    ///
    /// Moves the cursor.
    ///
    /// @param offset   the offset
    /// @param mode     the seek mode
    ///
    int seek(int offset, int mode);
};

#endif // IA32_INTERPRETER_SCANNER_H

