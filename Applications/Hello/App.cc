/*
 *
 *  Copyright (C) 2007, Waseda University.
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///
/// @file   Applications/Hello/App.cc
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @since  2007
/// 

//$Id: App.cc 383 2008-08-28 06:49:02Z hro $

/*
#include <Types.h>
#include <l4/schedule.h>
#include <l4/types.h>
#include <arc/driver.h>

#include <stdio.h>
#include <arc/parallel.h>

static int global_counter IS_PERSISTENT = 0;

static inline void
Speedo(int n)
{
    static char twiddlers[] = {'-', '\\', '|', '/'};
    printf("\b%c", twiddlers[n & 3]);
}

int
main(int state)
{
    printf("Hello, World! %d\n", state);

    int counter = 0;
    
    parallel_write("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 27);

    printf("Global Counter: %d\n", global_counter);
    printf("Local Counter: %d\n", counter);

    for (counter = 0; counter < 100000; counter++) {
        if (counter % 10000 == 0) {
            Speedo(counter / 10000);
        }
        global_counter++;
        L4_Yield();
    }

    return EXIT_RESTART;
}
*/

#include <Debug.h>
#include <System.h>
#include <l4/thread.h>

int
main(int argc, char* argv[])
{
    System.Print("(%.8lX) Hello, world!\n", L4_Myself().raw);
    if (argc > 1) {
        System.Print("(%.8lX) Message '%s'\n", L4_Myself().raw, argv[1]);
    }
    return 0;
}

