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

//$Id: fi_instruction_handler.cpp 388 2008-08-30 05:07:59Z hro $

#include "fi_instruction_handler.h"
#include "fi_opcode_handler.h"
#include "fi_scanner.h"

Int
instruction_handler::skip_zero()
{
    return _scanner.seek(0, scanner::SEEK_CUR);
}

Int
instruction_handler::skip_one()
{
    return _scanner.seek(1, scanner::SEEK_CUR);
}

Int
instruction_handler::skip_two()
{
    return _scanner.seek(2, scanner::SEEK_CUR);
}

Int
instruction_handler::skip_word(size_t word_size)
{
    return _scanner.seek(word_size, scanner::SEEK_CUR);
}

Int
instruction_handler::skip_modrm(UByte modrm, size_t word_size)
{
    Int ret;

    switch (decode_modrm_mod(modrm)) {
        case 0:
            if (word_size == 2) {
                if (decode_modrm_rm(modrm) == 6) {
                    ret = skip_two();
                }
                else {
                    ret = skip_zero();
                }
            }
            else if (word_size == 4) {
                switch (decode_modrm_rm(modrm)) {
                    case 4:
                        ret = skip_one();
                        break;
                    case 5:
                        ret = skip_word(word_size);
                        break;
                    default:
                        ret = skip_zero();
                }
            }
            else {
                ret = -1;
            }
            break;
        case 1:
            if (word_size == 4 && decode_modrm_rm(modrm) == 4) {
                // Skip the SIB byte
                ret = skip_two();
            }
            else {
                ret = skip_one();
            }
            break;
        case 2:
            if (word_size == 4 && decode_modrm_rm(modrm) == 4) {
                // Skip the SIB byte
                skip_one();
                ret = skip_word(word_size);
            }
            else {
                ret = skip_word(word_size);
            }
            break;
        case 3:
            ret = skip_zero();
            break;
        default:
            ret =-1;
    }
    return ret;
}


void
instruction_handler::skip_jcc(UByte opcode, size_t word_size)
{
    if ((0x70 <= opcode && opcode <= 0x7F) || opcode == 0xE3) {
        skip_one();
    }
    else {
        // 0x0F 0x81 <= opcode && opcode <= 0x0F 0x8F
        skip_one();
        skip_word(word_size);
    }
}

void
instruction_handler::skip_setcc(UByte opcode, size_t word_size)
{
    skip_two();
}

void
instruction_handler::skip_mov(UByte opcode, size_t word_size)
{
    //System.Print("skip_mov opcode: %x wsize: %d\n", opcode, word_size);
    switch (opcode) {
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8E:
        {
            UByte   next;
            Int     ret;

            _scanner.get(&next);
            ret = skip_modrm(next, word_size);
            break;
        }
        case 0xC6:
        {
            UByte   next;
            Int     ret;

            _scanner.get(&next);
            ret = skip_modrm(next, word_size);
            ret = skip_one();
            break;
        }
        case 0xC7:
        {
            UByte   next;
            Int     ret;

            _scanner.get(&next);
            ret = skip_modrm(next, word_size);
            ret = skip_word(word_size);
            break;
        }
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        {
            skip_word(word_size);
            break;
        }
        default:
        {
            if (0xB0 <= opcode && opcode <= 0xB7) {
                skip_one();
            }
            else if (0xB8 <= opcode && opcode <= 0xBF) {
                skip_word(word_size);
            }
            else {
                // unknown
            }
        }
    }
}

void
instruction_handler::skip_movsx(UByte opcode, size_t word_size)
{
    UByte modrm;
    // Skip BE or BF
    skip_one();
    // Get the MOD R/M byte
    _scanner.get(&modrm);
    skip_modrm(modrm, word_size);
}

void
instruction_handler::skip_movzx(UByte opcode, size_t word_size)
{
    UByte modrm;
    // Skip B6 or B7
    skip_one();
    // Get the MOD R/M byte
    _scanner.get(&modrm);
    skip_modrm(modrm, word_size);
}

void
instruction_handler::skip_lea(UByte opcode, size_t word_size)
{
    UByte modrm;
    _scanner.get(&modrm);
    skip_modrm(modrm, word_size);
}

