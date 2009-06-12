/*
 *
 *  Copyright (C) 2007, Waseda University. All rights reserved.
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
///
/// @file   Libraries/Arc/parallel.cc
/// @author Alexandre Courbot <alex@cs.waseda.jp>
/// @date   2008
///

// $Id: Parallel.cpp 349 2008-05-29 01:54:02Z hro $

#include <Parallel.h>
#include <Ipc.h>
#include <NameService.h>
#include <String.h>

static L4_ThreadId_t parallelId = {raw: 0};

static stat_t
getParallelServerID()
{
    if (parallelId.raw) return ERR_NONE;
    stat_t err;
    if ((err = NameService::Get("parallel", &parallelId)) != ERR_NONE) return err;
    return ERR_NONE;
}

/*
 * We will only use the MRs to transfer data here. This means we can use 62
 * MRs for transferring data itself (the amount of data transferred being stored
 * into MR1). This function therefore cuts the data to be sent in chunks of 62 MRs
 * and sends them to the parallel server.
 */
stat_t
parallel_write(const char *const data, size_t count) {
    stat_t err;
    // First make sure we can communicate with the parallel server
    err = getParallelServerID();
    if (err != ERR_NONE) return err;
    
    static const L4_Word_t nbRegsUsed = 62;
    static const L4_Word_t maxSizeOfIpc = nbRegsUsed * sizeof(L4_Word_t);
    // Number of IPC we will have to perform
    // We can send nbRegsUsed words along with every IPC
    int nbIpc = count / maxSizeOfIpc;
    if (count % maxSizeOfIpc) nbIpc++;
    for (int i = 0; i < nbIpc; i++) {
        L4_Word_t buf[nbRegsUsed + 1];
        L4_Word_t startIndex = i * maxSizeOfIpc;
        // Number of bytes to transfer
        L4_Word_t bytesToCopy = count - startIndex;
        if (bytesToCopy > maxSizeOfIpc) bytesToCopy = maxSizeOfIpc;
        // Number of registers to use for transfer
        L4_Word_t regsToCopy = bytesToCopy / sizeof(L4_Word_t);
        if (bytesToCopy % sizeof(L4_Word_t)) regsToCopy++;
        
        buf[0] = bytesToCopy;
        
        memcpy(&buf[1], data + startIndex, bytesToCopy);
        
        L4_Msg_t msg;
        L4_MsgPut(&msg, MSG_PARALLEL_WRITE, regsToCopy + 1, buf, 0, 0);
        Ipc::Call(parallelId, &msg, &msg);
    }
    return ERR_NONE;
}
