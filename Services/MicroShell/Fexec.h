#ifndef ARC_MICRO_SHELL_FEXEC_COMMAND_H
#define ARC_MICRO_SHELL_FEXEC_COMMAND_H

#include "Command.h"
#include <Types.h>
#include <String.h>
#include <l4/types.h>
#include <Ipc.h>
#include <System.h>

class FexecCommand : public Command
{
private:
    static const char*  NAME;

public:
    virtual Bool Match(const char* str, size_t len)
    { return strncmp(str, NAME, strlen(NAME)) == 0; }

    virtual stat_t Execute(int argc, char* argv[])
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
};

#endif // ARC_MICRO_SHELL_FEXEC_COMMAND_H
