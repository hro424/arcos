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

//$Id: fi_parser.cpp 427 2008-11-01 01:42:45Z hro $

#include <Types.h>
#include <Random.h>
#include <System.h>
#include "fi_parser.h"
#include "fi_instruction_handler.h"
#include "fi_opcode_handler.h"
#include "fi_scanner.h"


static inline void
ReadTSC(unsigned long* hi, unsigned long* lo)
{
    asm volatile ("rdtsc" : "=a" (*lo), "=d" (*hi));
}

void
parser::initialize()
{
    UInt seed, dummy;
    ReadTSC(&dummy, &seed);
    srand(seed);
    _counter = rand();
}

Int
parser::execute(scanner& s, instruction_handler* h, UInt freq)
{
    int             pos;
    unsigned char   c;

    while ((pos = s.get(&c)) >= 0) {
        //System.Print("0x%.8x:\t%.2x\n", static_cast<unsigned int>(pos), c);
        if (opcode_handler[static_cast<int>(c)] != 0) {
            Bool inject;

            // Control the frequency of the fault injection
            if (_counter % freq == 0) {
                inject = TRUE;
                System.Print("FI @ %.8lX\n", pos);
            }
            else {
                inject = FALSE;
            }

            // Invoke the opcode handler
            if (opcode_handler[static_cast<int>(c)](c, s, h, inject) < 0) {
                // Invalid opcode or EOF
                return -1;
            }
        }
        _counter++;
    }
    return 0;
} 

