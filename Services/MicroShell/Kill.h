#ifndef ARC_MICRO_SHELL_KILL_COMMAND_H
#define ARC_MICRO_SHELL_KILL_COMMAND_H

#include "Command.h"
#include <Types.h>
#include <String.h>

class KillCommand : public Command
{
private:
    static const char*  NAME;
public:
    virtual Bool Match(const char* str, size_t len)
    { return (strncmp(str, NAME, strlen(NAME)) == 0); }

    virtual stat_t Execute(int argc, char* argv[]) { return ERR_UNKNOWN; }
};

#endif // ARC_MICRO_SHELL_KILL_COMMAND_H
