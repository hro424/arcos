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
/// @brief  A command line parser
/// @file   Applications/MicroShell/Parser.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Parser.cc 386 2008-08-30 05:07:57Z hro $

#include <List.h>
#include "Command.h"
#include "Parser.h"

void
Parser::Parse(char* line)
{
    Iterator<Command*>& it = _command_list.GetIterator();

    // Skip the whitespaces at the beginning
    char* comstr = line;
    while (*comstr == ' ' || *comstr == '\t') {
        comstr++;
    }
    line = comstr;

    char* ptr = comstr;
    size_t comlen = 0;
    while ((*ptr != ' ') && (*ptr != '\t') && (*ptr != '\0')) {
        comlen++;
        ptr++;
    }

    if (*ptr != '\0') {
        *ptr = '\0';
        ptr++;
    }

    while (it.HasNext()) {
        Command *com = it.Next();
        if (com->Match(comstr, comlen)) {
            _current = com;
            ParseArgs(comstr, ptr);
            return;
        }
    }

    // Not found
    _current = &_unknown;
}

void
Parser::ParseArgs(char* command, char* args)
{
    _argv[0] = command;
    _argc = 1;
    char* ptr = args;
    while (*ptr != '\0') {
        while (*ptr == ' ') {
            *ptr = '\0';
            ptr++;
        }
        if (*ptr != '\0') {
            _argv[_argc] = ptr;
            _argc++;
            while (*ptr != ' ' && *ptr != '\0') {
                ptr++;
            }
        }
    }
}

