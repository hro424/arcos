#ifndef ARC_MICRO_SHELL_FREE_COMMAND_H
#define ARC_MICRO_SHELL_FREE_COMMAND_H

#include "Command.h"
#include <String.h>
#include <System.h>
#include <Ipc.h>

class FreeCommand : public Command
{
private:
    static const char* NAME;

public:
    virtual Bool Match(const char* str, size_t len)
    {
        return (strncmp(str, NAME, strlen(NAME)) == 0);
    }

    virtual stat_t Execute(int argc, char* argv[])
    {
        stat_t      err;
        L4_Msg_t    msg;

        L4_Put(&msg, MSG_ROOT_FREE_COUNT, 0, 0, 0, 0);
        err = Ipc::Call(L4_Pager(), &msg, &msg);
        if (err != ERR_NONE) {
            return err;
        }

        System.Print("Total     Free\n");
        System.Print("%8ld  %8ld\n", L4_Get(&msg, 1), L4_Get(&msg, 0));

        return ERR_NONE;
    }
};

#endif // ARC_MICRO_SHELL_FREE_COMMAND_H
