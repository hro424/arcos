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

//$Id: fi_instruction_handler.h 388 2008-08-30 05:07:59Z hro $

#ifndef IA32_INTERPRETER_INSTRUCTION_HANDLER_H
#define IA32_INTERPRETER_INSTRUCTION_HANDLER_H

#include <Types.h>
#include "fi_scanner.h"

class instruction_handler
{
protected:
    scanner&    _scanner;

    UByte decode_modrm_mod(UByte code) {
        return code >> 6;
    }

    UByte decode_modrm_reg(UByte code) {
        return (code >> 3) & 0x7;
    }

    UByte decode_modrm_rm(UByte code) {
        return code & 0x7;
    }

    Int skip_zero();
    Int skip_one();
    Int skip_two();
    Int skip_word(size_t word_size);
    Int skip_modrm(UByte modrm, size_t word_size);
    virtual void skip_jcc(UByte opcode, size_t word_size);
    virtual void skip_setcc(UByte opcode, size_t word_size);
    virtual void skip_mov(UByte opcode, size_t word_size);
    virtual void skip_movsx(UByte opcode, size_t word_size);
    virtual void skip_movzx(UByte opcode, size_t word_size);
    virtual void skip_lea(UByte opcode, size_t word_size);

public:
    instruction_handler(scanner& s) : _scanner(s) {}

    virtual void handle_jcc(UByte opcode, size_t word_size) {
        skip_jcc(opcode, word_size);
    }

    virtual void handle_setcc(UByte opcode, size_t word_size) {
        skip_setcc(opcode, word_size);
    }

    virtual void handle_mov(UByte opcode, size_t word_size) {
        skip_mov(opcode, word_size);
    }

    virtual void handle_movsx(UByte opcode, size_t word_size) {
        skip_movsx(opcode, word_size);
    }

    virtual void handle_movzx(UByte opcode, size_t word_size) {
        skip_movzx(opcode, word_size);
    }

    virtual void handle_lea(UByte opcode, size_t word_size) {
        skip_lea(opcode, word_size);
    }
};

#endif // IA32_INTERPRETER_INSTRUCTION_HANDLER_H

