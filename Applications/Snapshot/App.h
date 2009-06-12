//$Id: App.h 349 2008-05-29 01:54:02Z hro $

#ifndef SNAPSHOT_APP_H
#define SNAPSHOT_APP_H

#include <arc/console.h>
#include <l4/schedule.h>

#define ENTER           printf(TXT_BRIGHT TXT_FG_YELLOW "Enter %s\n"   \
                               TXT_NORMAL, __func__)
#define EXIT            printf(TXT_BRIGHT TXT_FG_YELLOW "Exit %s\n"    \
                               TXT_NORMAL, __func__)

#define SBUFSZ          10

static inline void
Speedo(int n)
{
    static char twiddlers[] = {'-', '\\', '|', '/'};
    printf("\b%c", twiddlers[n & 3]);
}

static inline void
Weight()
{
    /*
    for (int i = 0; i < 100000; i++) {
        if (i % 10000 == 0) {
            Speedo(i / 10000);
        }
        L4_Yield();
    }
    printf("\n");
    */
}

void TestStack3();
void TestStack4();

#endif // SNAPSHOT_APP_H
