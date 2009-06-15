#ifndef ARC_MICRO_SHELL_PS_COMMAND_H
#define ARC_MICRO_SHELL_PS_COMMAND_H

#include "Command.h"
#include <String.h>
#include <NameService.h>
#include <Types.h>

class PsCommand : public Command
{
private:
    static const char*  NAME;
public:
    virtual Bool Match(const char* str, size_t len)
    {
        Bool ret;
        ret = (strncmp(str, NAME, strlen(NAME)) == 0);
        return ret;
    }

    virtual stat_t Execute(int argc, char* argv[])
    {
        // Connect to the root NS
        // Get the list of running processes
        // Diplay the list

        return NameService::List();
    }
};

#endif // ARC_MICRO_SHELL_PS_COMMAND_H
