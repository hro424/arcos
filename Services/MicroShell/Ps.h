#ifndef ARC_MICRO_SHELL_PS_COMMAND_H
#define ARC_MICRO_SHELL_PS_COMMAND_H

#include "Command.h"
#include <String.h>
#include <NameService.h>
#include <Types.h>
#include <System.h>

class PsCommand : public Command
{
private:
    static const char*  NAME;
public:
    virtual Bool Match(const char* str, size_t len)
    {
        return (strncmp(str, NAME, strlen(NAME)) == 0);
    }

    virtual stat_t Execute(int argc, char* argv[])
    {
        // Connect to the root NS
        // Get the list of running processes
        // Diplay the list
        UInt i = 0;
        NameEntry entry;
        
        System.Print("TID     \tPAGER   \tNAME\n");
        while (NameService::Enumerate(i, &entry) == ERR_NONE) {
            System.Print("%.8lX\t%.8lX\t%s\n",
                         entry.tid.raw, entry.pager.raw, entry.name);
            i++;
        }

        return ERR_NONE;
    }
};

#endif // ARC_MICRO_SHELL_PS_COMMAND_H
