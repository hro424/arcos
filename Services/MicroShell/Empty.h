#ifndef ARC_MICRO_SHELL_EMPTY_COMMAND_H
#define ARC_MICRO_SHELL_EMPTY_COMMAND_H

#include "Command.h"
#include <System.h>
#include <Types.h>

///
/// Pseudo command for empty inputs
///
class EmptyCommand : public Command
{
public:
    virtual Bool Match(const char *str, size_t len)
    { return (*str == '\0') ? TRUE : FALSE; }

    virtual stat_t Execute(int argc, char *argv[])
    {
        System.Write("\n", 1);
        return ERR_NONE;
    }
};

#endif // ARC_MICRO_SHELL_EMPTY_COMMAND_H
