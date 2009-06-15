#ifndef ARC_MICRO_SHELL_EXIT_COMMAND
#define ARC_MICRO_SHELL_EXIT_COMMAND

#include "Command.h"
#include <String.h>
#include <Types.h>

class ExitCommand : public Command
{
private:
    static const char *NAME1;
    static const char *NAME2;

public:
    virtual Bool Match(const char *str, size_t len)
    {
        return (strncmp(str, NAME1, strlen(NAME1)) == 0) ||
            (strncmp(str, NAME2, strlen(NAME2)) == 0);
    }

    virtual stat_t Execute(int argc, char *argv[]) { return ERR_UNKNOWN; }
};

#endif // ARC_MICRO_SHELL_EXIT_COMMAND
