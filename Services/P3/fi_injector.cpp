/*
 *
 *  Copyright (C) 2008-2009, Waseda University.
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
/// @brief  Fault injector
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///


#include <Types.h>
#include <Random.h>
#include <System.h>
#include "Task.h"
#include "fi_injector.h"
#include "fi_parser.h"
#include "fi_scanner.h"

//
// @see Intel 64 and IA-32 Architectures Software Developer's Manual,
//      Volume 2X: Instruction Set Reference
//

class fi_base : public instruction_handler
{
protected:
    UByte random_8();
    UInt random_word(size_t word_size);
    UByte modify_reg(UByte modrm);
    UByte modify_rm_reg(UByte modrm);
public:
    fi_base(scanner& s) : instruction_handler(s) {}

};

UByte
fi_base::random_8() {
    return static_cast<UByte>(rand());
}

UInt
fi_base::random_word(size_t word_size) {
    if (word_size == 4) {
        return static_cast<UInt>(rand());
    }
    else {
        return static_cast<UInt>(rand()) & 0xFFFF;
    }
}

UByte
fi_base::modify_reg(UByte modrm) {
    UByte fault = modrm;
    while (fault == modrm) {
        fault = (modrm & 0xC7) | (static_cast<UByte>(rand()) & 0x38);
    }
    System.Print("FI: Modify REG %.2lX -> %.2lX\n", modrm, fault);
    return fault;
}

UByte
fi_base::modify_rm_reg(UByte modrm) {
    UByte fault = modrm;    // 0x00 - 0x3F
    while (fault == modrm) {
        fault = (modrm & 0xC0) | (static_cast<UByte>(rand()) & 0x3F);
    }
    System.Print("FI: Modify RM & REG %.2lX -> %.2lX\n", modrm, fault);
    return fault;
}


class fi_null : public fi_base
{
public:
    fi_null(scanner& s) : fi_base(s) {}

    virtual void handle_jcc(UByte opcode, size_t word_size) {
        //System.Print("fi_null jcc: %x\n", opcode);
        skip_jcc(opcode, word_size);
    }

    virtual void handle_setcc(UByte opcode, size_t word_size) {
        //System.Print("fi_null setcc: %x\n", opcode);
        skip_setcc(opcode, word_size);
    }

    virtual void handle_mov(UByte opcode, size_t word_size) {
        //System.Print("fi_null mov: %x\n", opcode);
        skip_mov(opcode, word_size);
    }

    virtual void handle_movsx(UByte opcode, size_t word_size) {
        //System.Print("fi_null movsx: %x\n", opcode);
        skip_movsx(opcode, word_size);
    }

    virtual void handle_movzx(UByte opcode, size_t word_size) {
        //System.Print("fi_null movzx: %x\n", opcode);
        skip_movzx(opcode, word_size);
    }

    virtual void handle_lea(UByte opcode, size_t word_size) {
        //System.Print("fi_null lea: %x\n", opcode);
        skip_lea(opcode, word_size);
    }
};


class fi_assignment : public fi_base
{
public:
    fi_assignment(scanner& s) : fi_base(s) {}

    virtual void handle_mov(UByte opcode, size_t word_size) {

        switch (opcode) {
            case 0x88:
            case 0x8A:
            case 0x89:
            case 0x8E:
            {
                //
                // Modify R/M and REG fields in the MOD R/M byte
                //

                UByte fault;
                UByte modrm;

                _scanner.get(&modrm);

                switch (decode_modrm_mod(modrm)) {
                    case 0:
                        if (word_size == 4 && decode_modrm_rm(modrm) == 4) {
                            fault = modify_reg(modrm);
                        }
                        else if (decode_modrm_rm(modrm) == 5) {
                            fault = modify_reg(modrm);
                        }
                        else {
                            fault = modify_rm_reg(modrm);
                        }
                        break;
                    case 1:
                        if (word_size == 4 && decode_modrm_rm(modrm) == 4) {
                            fault = modify_reg(modrm);
                        }
                        else {
                            fault = modify_rm_reg(modrm);
                        }
                        break;
                    case 2:
                        if (word_size == 4 && decode_modrm_rm(modrm) == 4) {
                            fault = modify_reg(modrm);
                        }
                        else {
                            fault = modify_rm_reg(modrm);
                        }
                        break;
                    case 3:
                        fault = modify_rm_reg(modrm);
                        break;
                    default:
                        System.Print(System.WARN, "Unknown modrm");
                        return;
                }

                // Inject a fault
                _scanner.seek(-1, scanner::SEEK_CUR);
                _scanner.put(fault);

                // OK. Let's work on the next instruction.
                skip_modrm(fault, word_size);

                break;
        }
        // [SKIP] Segment registers
        case 0x8B:
        case 0x8C:
        {
            UByte   next;
            _scanner.get(&next);
            skip_modrm(next, word_size);
            break;
        }
        // [SKIP] Immediate
        case 0xC6:
        {
            UByte   next;
            _scanner.get(&next);
            skip_modrm(next, word_size);
            skip_one();
            break;
        }
        // [SKIP] Immediate
        case 0xC7:
        {
            UByte   next;
            _scanner.get(&next);
            skip_modrm(next, word_size);
            skip_word(word_size);
            break;
        }
        // [SKIP] 'A' register (i.e. al, ax, eax)
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
            // Move imm8 to r8
            if (0xB0 <= opcode && opcode <= 0xB7) {
                UByte fault = opcode;
                while (fault == opcode) {
                    fault = 0xB0 | (static_cast<UByte>(rand()) & 0xF);
                }

                // Inject a fault
                _scanner.seek(-1, scanner::SEEK_CUR);
                _scanner.put(fault);

                // OK. Work on the next instruction.
                skip_one();

                System.Print("FI: MOV(%.2X): Change opcode %.2X\n",
                             opcode, fault);
            }
            // Move imm16/32 to r16/32
            else if (0xB8 <= opcode && opcode <= 0xBF) {
                UByte fault = opcode;
                while (fault == opcode) {
                    fault = 0xB8 | (static_cast<UByte>(rand()) & 0xF);
                }

                // Inject a fault
                _scanner.seek(-1, scanner::SEEK_CUR);
                _scanner.put(fault);

                // OK. Work on the next instruction.
                skip_word(word_size);

                System.Print("FI: MOV(%.2X): Change opcode %.2X\n",
                             opcode, fault);
            }
        }
        } // switch
    }

    virtual void handle_movsx(UByte opcode, size_t word_size) {
        UByte fault;
        UByte modrm;

        skip_one();
        _scanner.get(&modrm);
        fault = modify_rm_reg(modrm);
        _scanner.seek(-1, scanner::SEEK_CUR);
        _scanner.put(fault);
        skip_modrm(fault, word_size);
    }

    virtual void handle_movzx(UByte opcode, size_t word_size) {
        UByte fault;
        UByte modrm;

        skip_one();
        _scanner.get(&modrm);
        fault = modify_rm_reg(modrm);
        _scanner.seek(-1, scanner::SEEK_CUR);
        _scanner.put(fault);
        skip_modrm(fault, word_size);
    }
};


class fi_conditional : public fi_base
{
public:
    fi_conditional(scanner& s) : fi_base(s) {}

    virtual void handle_jcc(UByte opcode, size_t word_size) {
        //
        // Change the condition
        //

        if (opcode == 0x0F) {
            _scanner.get(&opcode);
        }

        if ((opcode & 0xF) % 2 == 0) {
            opcode++;
        }
        else {
            opcode--;
        }

        _scanner.seek(-1, scanner::SEEK_CUR);
        _scanner.put(opcode);

        skip_word(word_size);
    }
};


class fi_initialization : public fi_base
{
public:
    fi_initialization(scanner& s) : fi_base(s) {}

    virtual void handle_mov(UByte opcode, size_t word_size) {
        //
        // Modify immediate only
        //

        switch (opcode) {
            // [SKIP] Registers
            case 0x88:
            case 0x89:
            case 0x8A:
            case 0x8B:
            case 0x8C:
            case 0x8E:
            {
                UByte   next;

                _scanner.get(&next);
                skip_modrm(next, word_size);
                break;
            }
            // Immediate 8
            case 0xC6:
            {
                UByte   next;
                UByte   fault;

                _scanner.get(&next);
                skip_modrm(next, word_size);
                fault = random_8();
                _scanner.put(fault);

                System.Print("FI: MOV(%2X): Change IMM to %.2X\n",
                             opcode, fault);
                break;
            }
            // Immediate 16/32
            case 0xC7:
            {
                UByte   next;
                UInt    fault;

                _scanner.get(&next);
                skip_modrm(next, word_size);
                fault = random_word(word_size);
                _scanner.write(reinterpret_cast<const char*>(&fault),
                               word_size);

                System.Print("FI: MOV(%.2X): Change IMM to %.8lX\n",
                             opcode, fault);
                break;
            }
            // [SKIP] 'A' registers
            case 0xA0:
            case 0xA1:
            case 0xA2:
            case 0xA3:
            {
                skip_word(word_size);
                break;
            }
            // Immediate
            default:
            {
                if (0xB0 <= opcode && opcode <= 0xB7) {
                    UByte fault = random_8();
                    _scanner.put(fault);

                    System.Print("FI: MOV(%2X): Change IMM to %.2X\n",
                                 opcode, fault);
                }
                else if (0xB8 <= opcode && opcode <= 0xBF) {
                    UInt fault = random_word(word_size);
                    _scanner.write(reinterpret_cast<const char*>(&fault),
                                   word_size);

                    System.Print("FI: MOV(%2X): Change IMM to %.8lX\n",
                                 opcode, fault);
                }
                else {
                    System.Print(System.WARN, "Unknown opcode\n");
                }
            }
        }
    }
};

class fi_pointer : public fi_base
{
public:
    fi_pointer(scanner& s) : fi_base(s) {}

    virtual void handle_lea(UByte opcode, size_t word_size) {
        //NOT IMPLEMENTED
        skip_lea(opcode, word_size);
    }
};

class fi_offbyone : public fi_base
{
public:
    fi_offbyone(scanner& s) : fi_base(s) {}

    virtual void handle_jcc(UByte opcode, size_t word_size) {
        UByte fault = opcode;

        if (opcode == 0x0F) {
            _scanner.get(&opcode);
        }

        switch (opcode & 0x0F) {
            // JA <-> JAE
            // JB <-> JBE
            // JNA <-> JNAE
            // JNB <-> JNBE
            case 0x02:
            case 0x03:
                fault = opcode + 4;
                break;
            case 0x06:
            case 0x07:
                fault = opcode - 4;
                break;
            // JG <-> JGE
            // JL <-> JLE
            // JNG <-> JNGE
            // JNL <-> JNLE
            case 0x0C:
            case 0x0D:
                fault = opcode + 2;
                break;
            case 0x0E:
            case 0x0F:
                fault = opcode - 2;
                break;
            default:
                goto exit;
        }

        _scanner.seek(-1, scanner::SEEK_CUR);
        _scanner.put(fault);
        System.Print("FI: JCC(%.2X) OFF-BY-ONE %.2X\n", opcode, fault);
exit:
        skip_word(word_size);
    }
};


///
/// Scans text
///
static scanner _scanner;

///
/// Parses instructions
///
static parser _parser;


stat_t
Inject(Task* t, UInt type, UInt freq)
{
    stat_t                  ret;
    instruction_handler*    generator;

    _scanner.initialize(reinterpret_cast<char*>(t->text.StartAddress()),
                        reinterpret_cast<char*>(t->text.EndAddress()));
    _parser.initialize();

    switch (type) {
        case FI_NULL:
            generator = new fi_null(_scanner);
            break;
        case FI_ASSIGN:
            // 1. Corrupt assignment statements
            generator = new fi_assignment(_scanner);
            break;
        case FI_COND:
            // 2. Corrupt conditional constructs
            generator = new fi_conditional(_scanner);
            break;
        case FI_INIT:
            // 3. Initialization fault
            generator = new fi_initialization(_scanner);
            break;
        case FI_POINTER:
            // 4. Pointer corruption
            generator = new fi_pointer(_scanner);
            break;
        case FI_ALLOC:
            // 5. Allocation management
            generator = new fi_null(_scanner);
            break;
        case FI_COPY:
            // 6. Copy overrun
            generator = new fi_null(_scanner);
            break;
        case FI_OFFBYONE:
            // 7. Off-by-one
            generator = new fi_offbyone(_scanner);
            break;
        case FI_RAND:
            // 8. Random deletion
            //generator = new fi_random(freq);
            generator = new fi_null(_scanner);
            break;
        /*
        case FI_SYNC:
            // Synchronization
            generator = new fi_null(_scanner);
            break;
        */
        default:
            return ERR_INVALID_ARGUMENTS;
    }

    if (_parser.execute(_scanner, generator, freq) < 0) {
        ret = ERR_UNKNOWN;
    }
    else {
        ret = ERR_NONE;
    }

    delete generator;

    return ret;
}

