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

//$Id: Parser.h 386 2008-08-30 05:07:57Z hro $

#ifndef ARC_MICRO_SHELL_PARSER_H
#define ARC_MICRO_SHELL_PARSER_H

#include <Types.h>
#include <List.h>
#include "Unknown.h"

class Command;
class UnknownCommand;

class Parser
{
private:
    List<Command*>  _command_list;
    Command*        _current;
    UnknownCommand  _unknown;
    Int             _argc;
    char**          _argv;
    Byte            _vector_buf[4096];

    void ParseArgs(char* command, char* args);

public:
    Parser();

    ///
    /// Registers the command object to the list.
    ///
    void Register(Command* com);

    ///
    /// Removes the command object from the list.
    ///
    void Deregister(Command* com);

    ///
    /// Parses the command line.
    ///
    void Parse(char* line);

    ///
    /// Obtains the current command object.
    ///
    Command* GetCommand() const;

    ///
    /// Obtains the number of arguments.
    ///
    Int ArgCount() const;

    ///
    /// Obtains the array of arguments.
    ///
    char **ArgStrings() const;
};

inline
Parser::Parser()
{
    _argv = reinterpret_cast<char **>(_vector_buf);
}

inline void
Parser::Register(Command* com)
{
    _command_list.Append(com);
}

inline void
Parser::Deregister(Command* com)
{
    _command_list.Remove(com);
}

inline Command*
Parser::GetCommand() const
{
    return _current;
}

inline Int
Parser::ArgCount() const
{
    return _argc;
}

inline char**
Parser::ArgStrings() const
{
    return _argv;
}

#endif // ARC_MICRO_SHELL_PARSER_H

