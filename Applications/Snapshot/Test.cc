#include <l4/kdebug.h>
#include <arc/string.h>
#include <arc/system.h>
#include "App.h"

void
TestStack3()
{
    ENTER;
    char buffer[SBUFSZ];
    //L4_KDB_Enter("TestStack3: called");
    CHECKPOINT();
    memset(buffer, 0x3, SBUFSZ);
    //Weight();
    //L4_KDB_Enter("TestStack3: call TestStack4");
    TestStack4();

    EXIT;
    //L4_KDB_Enter("TestStack3: return");
}


