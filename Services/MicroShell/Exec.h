#ifndef ARC_MICRO_SHELL_EXEC_COMMAND_H
#define ARC_MICRO_SHELL_EXEC_COMMAND_H

#include "Command.h"
#include <FileStream.h>
#include <String.h>
#include <Types.h>
#include <NameService.h>
#include <MemoryAllocator.h>

class ExecCommand : public Command
{
private:
    FileStream      _fs;
    char*           _command_name;

public:
    virtual Bool Match(const char *str, size_t len)
    {
        Bool            result = FALSE;
        stat_t          err;
        L4_ThreadId_t   server;
        char*           path;
        char*           fs_name;

        path = static_cast<char*>(malloc(len + 1));
        memcpy(path, str, len + 1);
        fs_name = path;
        for (size_t i = 0; i < len + 1; i++) {
            if (path[i] == ':') {
                path[i] = '\0';
                path = &path[i + 1];
                break;
            }
        }

        if (fs_name == path) {
            //fs_name = DEFAULT_SERVER;
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

    virtual stat_t Execute(int argc, char *argv[])
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

};

#endif // ARC_MICRO_SHELL_EXEC_COMMAND_H
