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
/// @file   Applications/DiskTest/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: App.cc 429 2008-11-01 02:24:02Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <NameService.h>
#include <Random.h>
#include <Session.h>
#include <String.h>
#include <System.h>
#include <Types.h>
#include <l4/types.h>
#include <l4/ipc.h>


#define SHDPG_COUNT     2           // shared page count
#define SECTOR_SIZE     512         // the size of a disk sector
#define BUFFER_SIZE     8192        // the size of buffer

static char testbuf[BUFFER_SIZE];

stat_t
Read(Session *session, UInt device, void *buffer, size_t count,
     L4_Word_t offset)
{
    L4_Word_t   reg[3];
    addr_t      address;
    stat_t      err;
    ENTER;

    reg[0] = device;
    reg[1] = offset / SECTOR_SIZE;
    reg[2] = count / SECTOR_SIZE;
    err = session->Get(reg, 3);
    if (err != ERR_NONE) {
        return err;
    }
    
    address = session->GetBaseAddress();
    memcpy(buffer, (void *)address, count);

    EXIT;
    return ERR_NONE;
}

stat_t
Write(Session *session, UInt device, const void *buffer, size_t count,
      L4_Word_t offset)
{
    L4_Word_t       reg[3];
    addr_t          address;
    stat_t          err;
    ENTER;

    address = session->GetBaseAddress();
    memcpy((void *)address, buffer, count);

    reg[0] = device;
    reg[1] = offset / SECTOR_SIZE;
    reg[2] = count / SECTOR_SIZE;
    err = session->Put(reg, 3);
    if (err != ERR_NONE) {
        return err;
    }

    EXIT;
    return ERR_NONE;
}

/*
static int
GenerateRandomBinary(int weight)
{
    int n = rand() / weight;

    return !(n > 0);
}

// For fault injection test
static stat_t
ReadFI(Session *session, UInt device, void *buffer, size_t count,
     L4_Word_t offset)
{
    L4_Word_t   reg[3];
    addr_t      address;
    stat_t      err;
    ENTER;

    reg[0] = GenerateRandomBinary(5);
    reg[1] = offset / SECTOR_SIZE;
    reg[2] = count / SECTOR_SIZE;
    err = session->Get(reg, 3);
    if (err != ERR_NONE) {
        return err;
    }
    
    address = session->GetBaseAddress();
    memcpy(buffer, (void *)address, count);

    EXIT;
    return ERR_NONE;
}
*/


int
main()
{
    L4_ThreadId_t   tid;
    Session         *ss;

    System.Print("@@@@@@@@@@@@@@@@@ Disk Test %.8lX/%.8lX @@@@@@@@@@@@@@@@@\n",
                 L4_Myself().raw, L4_Pager().raw);

    NameService::Get("pata", &tid);
    DOUT("Disk server: %.8lX\n", tid.raw);
    if (L4_IsThreadEqual(tid, L4_nilthread)) {
        System.Print("Disk not found\n");
        return 1;
    }

    // Create a shared space
    ss = new Session();
    if (ss == 0) {
        return 1;
    }

    ss->Connect(tid);

    if (ss->IsConnected()) {
        DOUT("new session was successfully created\n");
    }
    else {
        delete ss;
        return 1;
    }

    for (;;) {
        Read(ss, 0, testbuf, BUFFER_SIZE, 0);
        /*
        for (int i = 0; i < 1024; i += 32) {
            for (int j = i; j < i + 32; j++) {
                System.Print("%.2X ", testbuf[j] & 0xFF);
            }
            System.Print("\n");
        }
        System.Print("\n");
        */
    }

    delete ss;

    // Return
    BREAK("End of Device Test");

    return 0;
}

