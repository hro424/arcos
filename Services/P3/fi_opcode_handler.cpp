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

//$Id: fi_opcode_handler.cpp 429 2008-11-01 02:24:02Z hro $

#include "fi_instruction_handler.h"
#include "fi_opcode_handler.h"
#include "fi_scanner.h"

#define DEFAULT_WORD_SIZE       4
static size_t _word_size = DEFAULT_WORD_SIZE;

#define DEFINE_OPCODE_HANDLER(n)                            \
    int _opcode_handler##n (unsigned char code,             \
                            scanner& input,                 \
                            instruction_handler* handler,   \
                            Bool inject)

#define OPCODE_HANDLER(n)           _opcode_handler##n

#define EMPTY_OPCODE_HANDLER(code)  0

#define REFINE_WORD_SIZE(v)         \
    if (v != DEFAULT_WORD_SIZE) {   \
        v = DEFAULT_WORD_SIZE;      \
    }

#define SKIP_ONE        skip_one(input)

#define SKIP_TWO        skip_two(input)

#define SKIP_WORD       skip_word(input, _word_size)

#define SKIP_MODRM      skip_modrm(input, _word_size)

#define OMIT_ONE        return SKIP_ONE
#define OMIT_TWO        return SKIP_TWO
#define OMIT_WORD       return SKIP_WORD
#define OMIT_MODRM      return SKIP_MODRM

#define DECODE_MODRM_MOD(code)  ((code) >> 6)
#define DECODE_MODRM_REG(code)  (((code) >> 3) & 0x7)
#define DECODE_MODRM_RM(code)   ((code) & 0x7)

static int
skip_zero(scanner& s)
{
    return s.seek(0, scanner::SEEK_CUR);
}

static int
skip_one(scanner& s)
{
    return s.seek(1, scanner::SEEK_CUR);
}

static int
skip_two(scanner& s)
{
    return s.seek(2, scanner::SEEK_CUR);
}

static int
_skip_word(scanner& s, size_t word_size)
{
    return s.seek(word_size, scanner::SEEK_CUR);
}

static int
skip_word(scanner& s, size_t& word_size)
{
    int ret = _skip_word(s, word_size);
    REFINE_WORD_SIZE(word_size);
    return ret;
}

static int
skip_instruction(int modrm, scanner& s, size_t& word_size)
{
    int ret;
    switch (DECODE_MODRM_MOD(modrm)) {
        case 0:
            if (word_size == 2) {
                if (DECODE_MODRM_RM(modrm) == 6) {
                    ret = skip_two(s);
                }
                else {
                    ret = skip_zero(s);
                }
            }
            else if (word_size == 4) {
                switch (DECODE_MODRM_RM(modrm)) {
                    case 4:
                        // Skip the SIB byte
                        ret = skip_one(s);
                        break;
                    case 5:
                        ret = skip_word(s, word_size);
                        break;
                    default:
                        ret = skip_zero(s);
                }
            }
            else {
                ret = -1;
            }
            break;
        case 1:
            if (word_size == 4 && DECODE_MODRM_RM(modrm) == 4) {
                // Skip the SIB byte
                ret = skip_two(s);
            }
            else {
                ret = skip_one(s);
            }
            break;
        case 2:
            if (word_size == 4 && DECODE_MODRM_RM(modrm) == 4) {
                // Skip the SIB byte
                skip_one(s);
                ret = skip_word(s, word_size);
            }
            else {
                ret = skip_word(s, word_size);
            }
            break;
        case 3:
            ret = skip_zero(s);
            break;
        default:
            ret = -1;
    }
    return ret;
}

static int
skip_modrm(scanner& s, size_t& word_size)
{
    unsigned char c;
    if (s.get(&c) < 0) {
        return -1;
    }
    return skip_instruction(c, s, word_size);
}


// ADD
DEFINE_OPCODE_HANDLER(00)
{
    OMIT_MODRM;
}

// ADD
DEFINE_OPCODE_HANDLER(01)
{
    OMIT_MODRM;
}

// ADD
DEFINE_OPCODE_HANDLER(03)
{
    OMIT_MODRM;
}

// ADD
DEFINE_OPCODE_HANDLER(05)
{
    OMIT_WORD;
}

// OR
DEFINE_OPCODE_HANDLER(08)
{
    OMIT_MODRM;
}

// OR
DEFINE_OPCODE_HANDLER(09)
{
    OMIT_MODRM;
}

// OR
DEFINE_OPCODE_HANDLER(0A)
{
    OMIT_MODRM;
}

// OR
DEFINE_OPCODE_HANDLER(0B)
{
    OMIT_MODRM;
}

// OR imm8
DEFINE_OPCODE_HANDLER(0C)
{
    OMIT_ONE;
}

// BSR, IMUL, JA, JAE, JB, JBE, JE, JG, JGE, JL, JLE, JNE, JS,
// MOVSBL, MOVZBL, MOVZWL, SETA, SETE, SETNE
DEFINE_OPCODE_HANDLER(0F)
{
    unsigned char c;
    if (input.get(&c) < 0) {
        return -1;
    }
    switch (c) {
        case 0x80: // JO
        case 0x81: // JNO
        case 0x82: // JB, JC, JNAE
        case 0x83: // JAE, JNB, JNC
        case 0x84: // JE, JZ
        case 0x85: // JNE, JNZ
        case 0x86: // JBE, JNA
        case 0x87: // JA, JNBE
        case 0x88: // JS
        case 0x89: // JNS
        case 0x8A: // JP
        case 0x8B: // JNP, JPO
        case 0x8C: // JL, JNGE
        case 0x8D: // JGE, JNL
        case 0x8E: // JLE, JNG
        case 0x8F: // JG, JNLE
            if (inject) {
                input.seek(-1, scanner::SEEK_CUR);
                handler->handle_jcc(0x0F, _word_size);
                REFINE_WORD_SIZE(_word_size);
            }
            else {
                SKIP_WORD;
            }
            break;
        case 0x94:  // SETE
        case 0x95:  // SETNE
        case 0x97:  // SETA
            if (inject) {
                input.seek(-1, scanner::SEEK_CUR);
                handler->handle_setcc(0x0F, _word_size);
                REFINE_WORD_SIZE(_word_size);
            }
            else {
                SKIP_ONE;
            }
            break;

        case 0xAF:  // IMUL
            SKIP_MODRM;
            break;

        case 0xB6:  // MOVZBL
        case 0xB7:  // MOVZWL
            if (inject) {
                input.seek(-1, scanner::SEEK_CUR);
                handler->handle_movzx(0x0F, _word_size);
                REFINE_WORD_SIZE(_word_size);
            }
            else {
                SKIP_MODRM;
            }
            break;
        case 0xBD:  // BSR -- Bit Scan Reverse
            SKIP_MODRM;
            break;
        case 0xBE:  // MOVSBL
            if (inject) {
                handler->handle_movsx(0x0F, _word_size);
                REFINE_WORD_SIZE(_word_size);
            }
            else {
                SKIP_MODRM;
            }
            break;

        default:
            return -1;
    }
    return 0;
}

// SBB
DEFINE_OPCODE_HANDLER(19)
{
    OMIT_MODRM;
}

// AND 8-bit
DEFINE_OPCODE_HANDLER(20)
{
    OMIT_MODRM;
}

// AND
DEFINE_OPCODE_HANDLER(21)
{
    OMIT_MODRM;
}

// AND
DEFINE_OPCODE_HANDLER(23)
{
    OMIT_MODRM;
}

// AND immediate
DEFINE_OPCODE_HANDLER(25)
{
    OMIT_WORD;
}

// SUB
DEFINE_OPCODE_HANDLER(29)
{
    OMIT_MODRM;
}

// SUB
DEFINE_OPCODE_HANDLER(2B)
{
    OMIT_MODRM;
}

// SUB immediate
DEFINE_OPCODE_HANDLER(2D)
{
    OMIT_WORD;
}

// XOR
DEFINE_OPCODE_HANDLER(31)
{
    OMIT_MODRM;
}

// CMP
DEFINE_OPCODE_HANDLER(38)
{
    OMIT_MODRM;
}

// CMP
DEFINE_OPCODE_HANDLER(39)
{
    OMIT_MODRM;
}

// CMP
DEFINE_OPCODE_HANDLER(3A)
{
    OMIT_MODRM;
}

// CMP
DEFINE_OPCODE_HANDLER(3B)
{
    OMIT_MODRM;
}

// CMP
DEFINE_OPCODE_HANDLER(3C)
{
    OMIT_ONE;
}

// CMP
DEFINE_OPCODE_HANDLER(3D)
{
    OMIT_WORD;
}

// PUSH
DEFINE_OPCODE_HANDLER(50)
{
    return 0;
}

// PUSH
DEFINE_OPCODE_HANDLER(51)
{
    return 0;
}

// PUSH
DEFINE_OPCODE_HANDLER(53)
{
    return 0;
}

// PUSH
DEFINE_OPCODE_HANDLER(55)
{
    return 0;
}

// PUSH
DEFINE_OPCODE_HANDLER(56)
{
    return 0;
}

// PUSH
DEFINE_OPCODE_HANDLER(57)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(58)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(59)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(5B)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(5D)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(5E)
{
    return 0;
}

// POP
DEFINE_OPCODE_HANDLER(5F)
{
    return 0;
}

// PUSHA
DEFINE_OPCODE_HANDLER(60)
{
    return 0;
}

// POPA
DEFINE_OPCODE_HANDLER(61)
{
    return 0;
}

// MOV??
DEFINE_OPCODE_HANDLER(65)
{
    handler->handle_mov(code, _word_size);
    REFINE_WORD_SIZE(_word_size);
    return 0;
}

DEFINE_OPCODE_HANDLER(66)
{
    unsigned char next;
    if (input.get(&next) < 0) {
        return -1;
    }

    if (next == 0x0F) { // mandatory prefix
    }
    else {
        // Switch 16/32-bit mode
        _word_size = 2;
    }
    return 0;
}

// JB
DEFINE_OPCODE_HANDLER(72)
{
    if (inject) {
        handler->handle_jcc(0x72, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JAE
DEFINE_OPCODE_HANDLER(73)
{
    if (inject) {
        handler->handle_jcc(0x73, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JE
DEFINE_OPCODE_HANDLER(74)
{
    if (inject) {
        handler->handle_jcc(0x74, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JNE
DEFINE_OPCODE_HANDLER(75)
{
    if (inject) {
        handler->handle_jcc(0x75, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JBE
DEFINE_OPCODE_HANDLER(76)
{
    if (inject) {
        handler->handle_jcc(0x76, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JA
DEFINE_OPCODE_HANDLER(77)
{
    if (inject) {
        handler->handle_jcc(0x77, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JS
DEFINE_OPCODE_HANDLER(78)
{
    if (inject) {
        handler->handle_jcc(0x78, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JNS
DEFINE_OPCODE_HANDLER(79)
{
    if (inject) {
        handler->handle_jcc(0x79, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JL
DEFINE_OPCODE_HANDLER(7C)
{
    if (inject) {
        handler->handle_jcc(0x7C, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JGE
DEFINE_OPCODE_HANDLER(7D)
{
    if (inject) {
        handler->handle_jcc(0x7D, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JLE
DEFINE_OPCODE_HANDLER(7E)
{
    if (inject) {
        handler->handle_jcc(0x7E, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// JG
DEFINE_OPCODE_HANDLER(7F)
{
    if (inject) {
        handler->handle_jcc(0x7F, 1);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// ADC, AND /4 ib, ANDB, CMP, CMPB /7 ib, OR /1 ib
DEFINE_OPCODE_HANDLER(80)
{
    int ret = SKIP_MODRM;
    if (ret < 0) {
        return ret;
    }
    return SKIP_ONE;
}

// ADD /0 iw, ADDL, AND /4 iw, CMP /7 iw, CMPL, OR /1 iw, SUB /5 iw
DEFINE_OPCODE_HANDLER(81)
{
    if (SKIP_MODRM >= 0 && SKIP_WORD >= 0) {
        return 0;
    }
    return -1;
}

// ADC /2 ib, ADD /0 ib, ADDL, AND /4 ib, CMP /7 ib, CMPL, OR /1 ib, SBB /3 ib, SBBL, SUB /5 ib, SUBL
DEFINE_OPCODE_HANDLER(83)
{
    if (SKIP_MODRM >= 0 && SKIP_ONE >= 0) {
        return 0;
    }
    return -1;
}

// TEST
DEFINE_OPCODE_HANDLER(84)
{
    OMIT_MODRM;
}

// TEST
DEFINE_OPCODE_HANDLER(85)
{
    OMIT_MODRM;
}

// MOV
DEFINE_OPCODE_HANDLER(88)
{
    if (inject) {
        handler->handle_mov(0x88, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(89)
{
    if (inject) {
        handler->handle_mov(0x89, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(8B)
{
    if (inject) {
        handler->handle_mov(0x8B, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
    }
    return 0;
}

// LEA
DEFINE_OPCODE_HANDLER(8D)
{
    if (inject) {
        handler->handle_lea(0x8D, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
    }
    return 0;
}

// NOP
DEFINE_OPCODE_HANDLER(90)
{
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(A1)
{
    if (inject) {
        handler->handle_mov(0xA1, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(A3)
{
    if (inject) {
        handler->handle_mov(0xA3, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// TEST
DEFINE_OPCODE_HANDLER(A8)
{
    OMIT_ONE;
}

// TEST
DEFINE_OPCODE_HANDLER(A9)
{
    OMIT_WORD;
}

// MOV
DEFINE_OPCODE_HANDLER(B0)
{
    if (inject) {
        handler->handle_mov(0xB0, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// MOV B0
DEFINE_OPCODE_HANDLER(B2)
{
    if (inject) {
        handler->handle_mov(0xB2, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(B8)
{
    if (inject) {
        handler->handle_mov(0xB8, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_ONE;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(B9)
{
    if (inject) {
        handler->handle_mov(0xB9, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(BA)
{
    if (inject) {
        handler->handle_mov(0xBA, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(BB)
{
    if (inject) {
        handler->handle_mov(0xBB, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(BE)
{
    if (inject) {
        handler->handle_mov(0xBE, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// MOV
DEFINE_OPCODE_HANDLER(BF)
{
    if (inject) {
        handler->handle_mov(0xBF, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_WORD;
    }
    return 0;
}

// SHR
DEFINE_OPCODE_HANDLER(C0)
{
    OMIT_TWO;
}

// SAR, SHL, SHLL, SHR
DEFINE_OPCODE_HANDLER(C1)
{
    unsigned char   modrm;
    int             ret;

    if (input.get(&modrm) < 0) {
        return -1;
    }

    switch (DECODE_MODRM_REG(modrm)) {
        case 4:
        case 5:
        case 7:
            switch (DECODE_MODRM_MOD(modrm)) {
                case 0:
                case 3:
                    ret = SKIP_ONE;
                    break;
                case 1:
                    ret = SKIP_TWO;
                    break;
                case 2:
                    ret = SKIP_WORD;
                    ret = SKIP_ONE;
                    break;
                default:
                    ret = -1;
            }
            break;
        default: ret = -1;
    }

    return ret;
}

// RET imm16
DEFINE_OPCODE_HANDLER(C2)
{
    OMIT_TWO;
}

// RET
DEFINE_OPCODE_HANDLER(C3)
{
    return 0;
}

// MOVB
DEFINE_OPCODE_HANDLER(C6)
{
    if (inject) {
        handler->handle_mov(0xC6, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
        SKIP_ONE;
    }
    return 0;
}

// MOVL
DEFINE_OPCODE_HANDLER(C7)
{
    if (inject) {
        handler->handle_mov(0xC7, _word_size);
        REFINE_WORD_SIZE(_word_size);
    }
    else {
        SKIP_MODRM;
        SKIP_ONE;
    }
    return 0;
}

// LEAVE
DEFINE_OPCODE_HANDLER(C9)
{
    return 0;
}

// INT3
DEFINE_OPCODE_HANDLER(CC)
{
    return 0;
}

// SHR
DEFINE_OPCODE_HANDLER(D1)
{
    OMIT_MODRM;
}

// SAR, SHL, SHR
DEFINE_OPCODE_HANDLER(D3)
{
    OMIT_MODRM;
}

// CALL
DEFINE_OPCODE_HANDLER(E8)
{
    OMIT_WORD;
}

// JMP
DEFINE_OPCODE_HANDLER(E9)
{
    OMIT_WORD;
}

// JMP
DEFINE_OPCODE_HANDLER(EB)
{
    OMIT_ONE;
}

// IN
DEFINE_OPCODE_HANDLER(EC)
{
    return 0;
}

// OUT
DEFINE_OPCODE_HANDLER(EE)
{
    return 0;
}

// LOCK
DEFINE_OPCODE_HANDLER(F0)
{
    OMIT_ONE;
}

// TESTB, TEST
DEFINE_OPCODE_HANDLER(F6)
{
    OMIT_MODRM;
}

// DIV, DIVL, MUL, MULL, NEG, NEGL, NOT, TEST, TESTL
DEFINE_OPCODE_HANDLER(F7)
{
    OMIT_MODRM;
}

// CALL, JMP, PUSHL
DEFINE_OPCODE_HANDLER(FF)
{
    OMIT_MODRM;
}


opcode_handler_t opcode_handler[0x100] =
{
    OPCODE_HANDLER(00),
    OPCODE_HANDLER(01),
    EMPTY_OPCODE_HANDLER(02),
    OPCODE_HANDLER(03),
    EMPTY_OPCODE_HANDLER(04),
    OPCODE_HANDLER(05),
    EMPTY_OPCODE_HANDLER(06),
    EMPTY_OPCODE_HANDLER(07),
    OPCODE_HANDLER(08),
    OPCODE_HANDLER(09),
    OPCODE_HANDLER(0A),
    OPCODE_HANDLER(0B),
    OPCODE_HANDLER(0C),
    EMPTY_OPCODE_HANDLER(0D),
    EMPTY_OPCODE_HANDLER(0E),
    OPCODE_HANDLER(0F),
    EMPTY_OPCODE_HANDLER(10),
    EMPTY_OPCODE_HANDLER(11),
    EMPTY_OPCODE_HANDLER(12),
    EMPTY_OPCODE_HANDLER(13),
    EMPTY_OPCODE_HANDLER(14),
    EMPTY_OPCODE_HANDLER(15),
    EMPTY_OPCODE_HANDLER(16),
    EMPTY_OPCODE_HANDLER(17),
    EMPTY_OPCODE_HANDLER(18),
    OPCODE_HANDLER(19),
    EMPTY_OPCODE_HANDLER(1A),
    EMPTY_OPCODE_HANDLER(1B),
    EMPTY_OPCODE_HANDLER(1C),
    EMPTY_OPCODE_HANDLER(1D),
    EMPTY_OPCODE_HANDLER(1E),
    EMPTY_OPCODE_HANDLER(1F),
    OPCODE_HANDLER(20),
    OPCODE_HANDLER(21),
    EMPTY_OPCODE_HANDLER(22),
    OPCODE_HANDLER(23),
    EMPTY_OPCODE_HANDLER(24),
    OPCODE_HANDLER(25),
    EMPTY_OPCODE_HANDLER(26),
    EMPTY_OPCODE_HANDLER(27),
    EMPTY_OPCODE_HANDLER(28),
    OPCODE_HANDLER(29),
    EMPTY_OPCODE_HANDLER(2A),
    OPCODE_HANDLER(2B),
    EMPTY_OPCODE_HANDLER(2C),
    OPCODE_HANDLER(2D),
    EMPTY_OPCODE_HANDLER(2E),
    EMPTY_OPCODE_HANDLER(2F),
    EMPTY_OPCODE_HANDLER(30),
    OPCODE_HANDLER(31),
    EMPTY_OPCODE_HANDLER(32),
    EMPTY_OPCODE_HANDLER(33),
    EMPTY_OPCODE_HANDLER(34),
    EMPTY_OPCODE_HANDLER(35),
    EMPTY_OPCODE_HANDLER(36),
    EMPTY_OPCODE_HANDLER(37),
    OPCODE_HANDLER(38),
    OPCODE_HANDLER(39),
    OPCODE_HANDLER(3A),
    OPCODE_HANDLER(3B),
    OPCODE_HANDLER(3C),
    OPCODE_HANDLER(3D),
    EMPTY_OPCODE_HANDLER(3E),
    EMPTY_OPCODE_HANDLER(3F),
    EMPTY_OPCODE_HANDLER(40),
    EMPTY_OPCODE_HANDLER(41),
    EMPTY_OPCODE_HANDLER(42),
    EMPTY_OPCODE_HANDLER(43),
    EMPTY_OPCODE_HANDLER(44),
    EMPTY_OPCODE_HANDLER(45),
    EMPTY_OPCODE_HANDLER(46),
    EMPTY_OPCODE_HANDLER(47),
    EMPTY_OPCODE_HANDLER(48),
    EMPTY_OPCODE_HANDLER(49),
    EMPTY_OPCODE_HANDLER(4A),
    EMPTY_OPCODE_HANDLER(4B),
    EMPTY_OPCODE_HANDLER(4C),
    EMPTY_OPCODE_HANDLER(4D),
    EMPTY_OPCODE_HANDLER(4E),
    EMPTY_OPCODE_HANDLER(4F),
    OPCODE_HANDLER(50),
    OPCODE_HANDLER(51),
    EMPTY_OPCODE_HANDLER(52),
    OPCODE_HANDLER(53),
    EMPTY_OPCODE_HANDLER(54),
    OPCODE_HANDLER(55),
    OPCODE_HANDLER(56),
    OPCODE_HANDLER(57),
    OPCODE_HANDLER(58),
    OPCODE_HANDLER(59),
    EMPTY_OPCODE_HANDLER(5A),
    OPCODE_HANDLER(5B),
    EMPTY_OPCODE_HANDLER(5C),
    OPCODE_HANDLER(5D),
    OPCODE_HANDLER(5E),
    OPCODE_HANDLER(5F),
    OPCODE_HANDLER(60),
    OPCODE_HANDLER(61),
    EMPTY_OPCODE_HANDLER(62),
    EMPTY_OPCODE_HANDLER(63),
    EMPTY_OPCODE_HANDLER(64),
    OPCODE_HANDLER(65),
    OPCODE_HANDLER(66),
    EMPTY_OPCODE_HANDLER(67),
    EMPTY_OPCODE_HANDLER(68),
    EMPTY_OPCODE_HANDLER(69),
    EMPTY_OPCODE_HANDLER(6A),
    EMPTY_OPCODE_HANDLER(6B),
    EMPTY_OPCODE_HANDLER(6C),
    EMPTY_OPCODE_HANDLER(6D),
    EMPTY_OPCODE_HANDLER(6E),
    EMPTY_OPCODE_HANDLER(6F),
    EMPTY_OPCODE_HANDLER(70),
    EMPTY_OPCODE_HANDLER(71),
    OPCODE_HANDLER(72),
    OPCODE_HANDLER(73),
    OPCODE_HANDLER(74),
    OPCODE_HANDLER(75),
    OPCODE_HANDLER(76),
    OPCODE_HANDLER(77),
    OPCODE_HANDLER(78),
    OPCODE_HANDLER(79),
    EMPTY_OPCODE_HANDLER(7A),
    EMPTY_OPCODE_HANDLER(7B),
    OPCODE_HANDLER(7C),
    OPCODE_HANDLER(7D),
    OPCODE_HANDLER(7E),
    OPCODE_HANDLER(7F),
    OPCODE_HANDLER(80),
    OPCODE_HANDLER(81),
    EMPTY_OPCODE_HANDLER(82),
    OPCODE_HANDLER(83),
    OPCODE_HANDLER(84),
    OPCODE_HANDLER(85),
    EMPTY_OPCODE_HANDLER(86),
    EMPTY_OPCODE_HANDLER(87),
    OPCODE_HANDLER(88),
    OPCODE_HANDLER(89),
    EMPTY_OPCODE_HANDLER(8A),
    OPCODE_HANDLER(8B),
    EMPTY_OPCODE_HANDLER(8C),
    OPCODE_HANDLER(8D),
    EMPTY_OPCODE_HANDLER(8E),
    EMPTY_OPCODE_HANDLER(8F),
    OPCODE_HANDLER(90),
    EMPTY_OPCODE_HANDLER(91),
    EMPTY_OPCODE_HANDLER(92),
    EMPTY_OPCODE_HANDLER(93),
    EMPTY_OPCODE_HANDLER(94),
    EMPTY_OPCODE_HANDLER(95),
    EMPTY_OPCODE_HANDLER(96),
    EMPTY_OPCODE_HANDLER(97),
    EMPTY_OPCODE_HANDLER(98),
    EMPTY_OPCODE_HANDLER(99),
    EMPTY_OPCODE_HANDLER(9A),
    EMPTY_OPCODE_HANDLER(9B),
    EMPTY_OPCODE_HANDLER(9C),
    EMPTY_OPCODE_HANDLER(9D),
    EMPTY_OPCODE_HANDLER(9E),
    EMPTY_OPCODE_HANDLER(9F),
    EMPTY_OPCODE_HANDLER(A0),
    OPCODE_HANDLER(A1),
    EMPTY_OPCODE_HANDLER(A2),
    OPCODE_HANDLER(A3),
    EMPTY_OPCODE_HANDLER(A4),
    EMPTY_OPCODE_HANDLER(A5),
    EMPTY_OPCODE_HANDLER(A6),
    EMPTY_OPCODE_HANDLER(A7),
    OPCODE_HANDLER(A8),
    OPCODE_HANDLER(A9),
    EMPTY_OPCODE_HANDLER(AA),
    EMPTY_OPCODE_HANDLER(AB),
    EMPTY_OPCODE_HANDLER(AC),
    EMPTY_OPCODE_HANDLER(AD),
    EMPTY_OPCODE_HANDLER(AE),
    EMPTY_OPCODE_HANDLER(AF),
    OPCODE_HANDLER(B0),
    EMPTY_OPCODE_HANDLER(B1),
    OPCODE_HANDLER(B2),
    EMPTY_OPCODE_HANDLER(B3),
    EMPTY_OPCODE_HANDLER(B4),
    EMPTY_OPCODE_HANDLER(B5),
    EMPTY_OPCODE_HANDLER(B6),
    EMPTY_OPCODE_HANDLER(B7),
    OPCODE_HANDLER(B8),
    OPCODE_HANDLER(B9),
    OPCODE_HANDLER(BA),
    OPCODE_HANDLER(BB),
    EMPTY_OPCODE_HANDLER(BC),
    EMPTY_OPCODE_HANDLER(BD),
    OPCODE_HANDLER(BE),
    OPCODE_HANDLER(BF),
    OPCODE_HANDLER(C0),
    OPCODE_HANDLER(C1),
    OPCODE_HANDLER(C2),
    OPCODE_HANDLER(C3),
    EMPTY_OPCODE_HANDLER(C4),
    EMPTY_OPCODE_HANDLER(C5),
    OPCODE_HANDLER(C6),
    OPCODE_HANDLER(C7),
    EMPTY_OPCODE_HANDLER(C8),
    OPCODE_HANDLER(C9),
    EMPTY_OPCODE_HANDLER(CA),
    EMPTY_OPCODE_HANDLER(CB),
    OPCODE_HANDLER(CC),
    EMPTY_OPCODE_HANDLER(CD),
    EMPTY_OPCODE_HANDLER(CE),
    EMPTY_OPCODE_HANDLER(CF),
    EMPTY_OPCODE_HANDLER(D0),
    OPCODE_HANDLER(D1),
    EMPTY_OPCODE_HANDLER(D2),
    OPCODE_HANDLER(D3),
    EMPTY_OPCODE_HANDLER(D4),
    EMPTY_OPCODE_HANDLER(D5),
    EMPTY_OPCODE_HANDLER(D6),
    EMPTY_OPCODE_HANDLER(D7),
    EMPTY_OPCODE_HANDLER(D8),
    EMPTY_OPCODE_HANDLER(D9),
    EMPTY_OPCODE_HANDLER(DA),
    EMPTY_OPCODE_HANDLER(DB),
    EMPTY_OPCODE_HANDLER(DC),
    EMPTY_OPCODE_HANDLER(DD),
    EMPTY_OPCODE_HANDLER(DE),
    EMPTY_OPCODE_HANDLER(DF),
    EMPTY_OPCODE_HANDLER(E0),
    EMPTY_OPCODE_HANDLER(E1),
    EMPTY_OPCODE_HANDLER(E2),
    EMPTY_OPCODE_HANDLER(E3),
    EMPTY_OPCODE_HANDLER(E4),
    EMPTY_OPCODE_HANDLER(E5),
    EMPTY_OPCODE_HANDLER(E6),
    EMPTY_OPCODE_HANDLER(E7),
    OPCODE_HANDLER(E8),
    OPCODE_HANDLER(E9),
    EMPTY_OPCODE_HANDLER(EA),
    OPCODE_HANDLER(EB),
    OPCODE_HANDLER(EC),
    EMPTY_OPCODE_HANDLER(ED),
    OPCODE_HANDLER(EE),
    EMPTY_OPCODE_HANDLER(EF),
    OPCODE_HANDLER(F0),
    EMPTY_OPCODE_HANDLER(F1),
    EMPTY_OPCODE_HANDLER(F2),
    EMPTY_OPCODE_HANDLER(F3),
    EMPTY_OPCODE_HANDLER(F4),
    EMPTY_OPCODE_HANDLER(F5),
    OPCODE_HANDLER(F6),
    OPCODE_HANDLER(F7),
    EMPTY_OPCODE_HANDLER(F8),
    EMPTY_OPCODE_HANDLER(F9),
    EMPTY_OPCODE_HANDLER(FA),
    EMPTY_OPCODE_HANDLER(FB),
    EMPTY_OPCODE_HANDLER(FC),
    EMPTY_OPCODE_HANDLER(FD),
    EMPTY_OPCODE_HANDLER(FE),
    OPCODE_HANDLER(FF)
};


