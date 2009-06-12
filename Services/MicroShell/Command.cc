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
/// @brief  Base class of shell commands
/// @file   Applications/MicroShell/Command.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008

//$Id: Command.cc 410 2008-09-08 12:00:15Z hro $

#include <FileStream.h>
#include <String.h>
#include <Types.h>
#include <MemoryAllocator.h>
#include <NameService.h>
#include "Command.h"

Bool
EmptyCommand::Match(const char *ptr, size_t len)
{
    return (*ptr == '\0') ? TRUE : FALSE;
}

stat_t
EmptyCommand::Execute(int argc, char *argv[])
{
    System.Write("\n", 1);
    return ERR_NONE;
}

const char* UnknownCommand::MESSAGE = "msh: unknown command\n";

Bool
UnknownCommand::Match(const char *ptr, size_t len)
{
    return TRUE;
}

stat_t
UnknownCommand::Execute(int argc, char *argv[])
{
    System.Print("%s", MESSAGE);
    return ERR_NOT_FOUND;
}

const char* ExitCommand::NAME1 = "exit";
const char* ExitCommand::NAME2 = "quit";

Bool
ExitCommand::Match(const char *ptr, size_t len)
{
    return (strncmp(ptr, NAME1, strlen(NAME1)) == 0) ||
        (strncmp(ptr, NAME2, strlen(NAME2)) == 0);
}

stat_t
ExitCommand::Execute(int argc, char *argv[])
{
    return ERR_UNKNOWN;
}

const char* HistoryCommand::NAME = "history";

Bool
HistoryCommand::Match(const char *ptr, size_t len)
{
    return (strncmp(ptr, NAME, strlen(NAME)) == 0);
}

stat_t
HistoryCommand::Execute(int argc, char *argv[])
{
    return ERR_NONE;
}

Bool
ExecCommand::Match(const char *ptr, size_t len)
{
    Bool            result = FALSE;
    stat_t          err;
    L4_ThreadId_t   server;
    char*           path;
    char*           fs_name;

    path = static_cast<char*>(malloc(len + 1));
    memcpy(path, ptr, len + 1);
    fs_name = path;
    for (size_t i = 0; i < len + 1; i++) {
        if (path[i] == ':') {
            path[i] = '\0';
            path = &path[i + 1];
            break;
        }
    }

    DOUT("fs '%s'\n", fs_name);
    err = NameService::Get(fs_name, &server);
    if (err != ERR_NONE) {
        return FALSE;
    }

    err = _fs.Connect(server);
    if (err != ERR_NONE) {
        return FALSE;
    }

    DOUT("open '%s'\n", path);
    err = _fs.Open(path, FileStream::READ);
    if (err == ERR_NONE) {
        DOUT("found '%s'\n", path);
        _fs.Close();
        result = TRUE;
    }
    _fs.Disconnect();

    mfree(fs_name);

    return result;
}

stat_t
ExecCommand::Execute(int argc, char* argv[])
{
    L4_Word_t   regs[2];
    L4_Msg_t    msg;
    stat_t      err;

    regs[0] = static_cast<L4_Word_t>(argc);
    regs[1] = reinterpret_cast<L4_Word_t>(argv);

    L4_Put(&msg, MSG_ROOT_EXEC, 2, regs, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    return err;
}


const char* FexecCommand::NAME = "fexec";

Bool
FexecCommand::Match(const char* ptr, size_t len)
{
    return strncmp(ptr, NAME, strlen(NAME)) == 0;
}

stat_t
FexecCommand::Execute(int argc, char* argv[])
{
    L4_Word_t   regs[2];
    L4_Msg_t    msg;
    stat_t      err;

    if (argc < 4) {
        System.Print("usage: fexec <type> <freq> <command> <args...>\n");
        return ERR_NONE;
    }

    regs[0] = static_cast<L4_Word_t>(argc);
    regs[1] = reinterpret_cast<L4_Word_t>(argv);

    L4_Put(&msg, MSG_ROOT_EXEC, 2, regs, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    return err;
}


const char* PsCommand::NAME = "ps";

Bool
PsCommand::Match(const char* ptr, size_t len)
{
    Bool ret;
    ret = (strncmp(ptr, NAME, strlen(NAME)) == 0);
    return ret;
}

stat_t
PsCommand::Execute(int argc, char* argv[])
{
    // Connect to the root NS
    // Get the list of running processes
    // Diplay the list

    return NameService::List();
}


const char* KillCommand::NAME = "kill";

Bool
KillCommand::Match(const char* ptr, size_t len)
{
    return (strncmp(ptr, NAME, strlen(NAME)) == 0);
}

stat_t
KillCommand::Execute(int argc, char* argv[])
{
    return ERR_UNKNOWN;
}


const char* ListCommand::NAME = "ls";

Bool
ListCommand::Match(const char* ptr, size_t len)
{
    return (strncmp(ptr, NAME, strlen(NAME)) == 0);
}

stat_t
ListCommand::Execute(int argc, char* argv[])
{
    struct Dir {
        UInt    inode;
        UShort  record_len;
        UByte   name_len;
        UByte   type;
        char    name[];
    };

    static char     buffer[4096];
    size_t          rsize;
    size_t          len;
    stat_t          err;
    Dir*            ptr;
    char*           path;
    char*           fs_name;
    L4_ThreadId_t   server;

    if (argc < 2) {
        return ERR_NOT_FOUND;
    }

    fs_name = argv[1];
    path = argv[1];
    len = strlen(path) + 1;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == ':') {
            path[i] = '\0';
            path = &path[i + 1];
            break;
        }
    }

    err = NameService::Get(fs_name, &server);
    if (err != ERR_NONE) {
        return err;
    }


    if (_fs.Connect(server) != ERR_NONE) {
        FATAL("fs not found");
    }

    err = _fs.Open(path, FileStream::READ);
    if (err != ERR_NONE) {
        return err;
    }

    _fs.Read(buffer, 4096, &rsize);
    _fs.Close();

    for (UInt i = 0; i < rsize;) {
        ptr = reinterpret_cast<Dir*>(buffer + i);
        System.Print("reclen %u nmlen %u ino %lu type %u str %s\n",
                     ptr->record_len, ptr->name_len, ptr->inode, ptr->type,
                     ptr->name);
        i += ptr->record_len;
    }

    _fs.Disconnect();

    return ERR_NONE;
}

