
///
/// @brief  Page fault performance evaluation
/// @file   Applications/PFTest/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: App.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/random.h>
#include <arc/space.h>
#include <arc/system.h>
#include <arc/types.h>
#include <stdio.h>

#define NEXPERIMENTS    1000
#define NPAGES          10

char    buffer[PAGE_SIZE * NPAGES]    __attribute__ ((aligned (PAGE_SIZE)));

int
main()
{
    char*   ptr;
    int     skipped = 0;
    int     experiments = NEXPERIMENTS;
    ULong   before;
    ULong   after;
    ULong   consume = 0;

    for (int i = 0; i < experiments; i++) {
        ptr = buffer + PAGE_SIZE * (random() % NPAGES);

        before = Rdtsc();
        *ptr = 0xAB;
        after = Rdtsc();
        Unmap(reinterpret_cast<addr_t>(ptr));

        if (after > before) {
            if (consume != 0) {
                    consume = (consume + (after - before)) / 2;
            }
            else {
                consume = after - before;
            }
        }
        else {
            experiments++;
        }
    }

    printf("count = %lld (%d times average)\n",
           consume, NEXPERIMENTS - skipped);

    return 0;
}
