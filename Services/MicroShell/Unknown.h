#ifndef ARC_MICRO_SHELL_UNKNOWN_COMMAND_H
#define ARC_MICRO_SHELL_UNKNOWN_COMMAND_H

#include "Command.h"
#include <System.h>

///
/// Processes unknown commands
///
class UnknownCommand : public Command
{
private:
    static const char *MESSAGE;
public:
    virtual Bool Match(const char *str, size_t len) { return TRUE; }

    virtual stat_t Execute(int argc, char *argv[])
    {
        System.Print("%s", MESSAGE);
        return ERR_NOT_FOUND;
    }
};

#endif // ARC_MICROSHELL_UNKNOWN_COMMAND_H
