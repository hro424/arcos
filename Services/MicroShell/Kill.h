#ifndef ARC_MICRO_SHELL_KILL_COMMAND_H
#define ARC_MICRO_SHELL_KILL_COMMAND_H

#include "Command.h"
#include <Types.h>
#include <String.h>

class KillCommand : public Command
{
private:
    L4_Word_t atox(char* str)
    {
        L4_Word_t   i;
        L4_Word_t   val;

        while (*str == ' ' || *str == '\t') {
            str++;
        }

        while (*str == '0') {
            str++;
        }

        i = 0;
        val = 0;
        while (*str != '\0' && i < 16) {
            if (('0' <= *str && *str <= '9')) {
                val *= 16;
                val += *str - '0';
                str++;
                i++;
            }
            else if (('a' <= *str && *str <= 'f')) {
                val *= 16;
                val += *str - 'a' + 10;
                str++;
                i++;
            }
            else if (('A' <= *str && *str <= 'F')) {
                val *= 16;
                val += *str - 'A' + 10;
                str++;
                i++;
            }
            else {
                return 0;
            }
        }

        return val;
    }

public:
    virtual Bool Match(const char* str, size_t len)
    {
        const char* NAME = "kill";
        return (strncmp(str, NAME, strlen(NAME)) == 0);
    }

    virtual stat_t Execute(int argc, char* argv[])
    {
        L4_Msg_t    msg;
        L4_Word_t   tid;
        stat_t      err;

        if (argc < 2) {
            return ERR_INVALID_ARGUMENTS;
        }

        tid = (L4_Word_t)atox(argv[1]);
        if (tid == 0) {
            return ERR_INVALID_ARGUMENTS;
        }

        L4_Put(&msg, MSG_ROOT_KILL, 1, &tid, 0, 0);
        err = Ipc::Call(L4_Pager(), &msg, &msg);
        if (err == ERR_INVALID_SPACE) {
            System.Print("task %x not found\n", tid);
        }

        return ERR_NONE;
    }
};

#endif // ARC_MICRO_SHELL_KILL_COMMAND_H
