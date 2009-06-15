#ifndef ARC_MICRO_SHELL_HISTORY_COMMAND_H
#define ARC_MICRO_SHELL_HISTORY_COMMAND_H

#include <String.h>
#include <Types.h>
#include "Command.h"

class HistoryCommand : public Command
{
private:
    static const char *NAME;

public:
    virtual Bool Match(const char *str, size_t len)
    { return (strncmp(str, NAME, strlen(NAME)) == 0); }

    virtual stat_t Execute(int argc, char *argv[]) { return ERR_NONE; }
};


#endif // ARC_MICRO_SHELL_HISTORY_COMMAND_H
