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
/// @brief  Micro shell for standalone server.
/// @file   Applications/MicroShell/Shell.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Shell.cc 410 2008-09-08 12:00:15Z hro $

#include <Debug.h>
#include <NameService.h>
#include <String.h>
#include <System.h>
#include "Command.h"
#include "Parser.h"

#define DEFAULT_FS  "ram"

int
main()
{
    static const char*      version = "Arc operating system, version 0.1\n";
    static const char*      message = "Welcome to Arc Micro Shell. Enjoy!\n";
    static const char*      prompt = "msh% ";

    static EmptyCommand     empty;
    static ExitCommand      exit;
    static HistoryCommand   history;
    static PsCommand        ps;
    static KillCommand      kill;
    static FexecCommand     fexec;
    static ExecCommand      exec;
    static ListCommand      list;

    Parser parser;

    parser.Register(&empty);
    parser.Register(&exit);
    parser.Register(&history);
    parser.Register(&list);
    parser.Register(&ps);
    parser.Register(&kill);
    parser.Register(&fexec);
    parser.Register(&exec);

    // Display the welcome message
    System.Print("%s", version);
    System.Print("%s", message);

    for (;;) {
        System.Write(prompt, strlen(prompt));
        System.Flush();

        // read user input
        //XXX: line may be broken.
        char *line = System.ReadLine();

        // parse it
        parser.Parse(line);

        Command *command = parser.GetCommand();

        // invoke a command
        stat_t ret = command->Execute(parser.ArgCount(), parser.ArgStrings());
        if (ret == ERR_UNKNOWN) {
            break;
        }
    }

    return 0;
}

