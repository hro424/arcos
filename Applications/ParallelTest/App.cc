
//$Id: App.cc 349 2008-05-29 01:54:02Z hro $

#include <arc/debug.h>
#include <arc/console.h>
#include <arc/system.h>
#include <arc/types.h>
#include <l4/schedule.h>
#include <l4/types.h>
#include <arc/driver.h>

#include <stdio.h>
#include <arc/parallel.h>

int
main()
{
    printf("Paralell driver test\n");

    for (int counter = 0; counter < 10000000; counter++) {
        if (counter % 10000 == 0) {
            parallel_write("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 27);
        }
        L4_Yield();
    }

    return EXIT_SUCCESS;
}
