/*
 *
 *  Copyright (C) 2008, Waseda University.
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
/// @file   Services/File/Ext2/Ext2FsServer.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: Ext2FsServer.cc 429 2008-11-01 02:24:02Z hro $

#include <arc/server.h>
#include <Disk.h>
#include <Ipc.h>
#include <Mutex.h>
#include <SelfHealingServer.h>
#include <String.h>
#include <l4/types.h>
#include <l4/message.h>
#include <l4/ipc.h>

#include "Ext2File.h"
#include "Ext2Fs.h"
#include "Ext2FsServer.h"
#include "Ext2Partition.h"
#include "Inode.h"


/*
struct Ext2Client : public SessionClient
{
    Ext2File*   file;

    Ext2Client(L4_ThreadId_t t, addr_t b, size_t s)
        : SessionClient(t, b, s), file(0) {}
};


void
Ext2FsServer::Register(const L4_ThreadId_t& tid, addr_t base, size_t size)
{
    Ext2Client* c = new Ext2Client(tid, base, size);
    if (c == 0) {
        FATAL("out of memory");
        return;
    }
    _clients.Append(c);
}
*/

static Byte         __index[SelfHealingSessionServer::NUM_CLIENTS] IS_PERSISTENT;
static Ext2File     __file_container[SelfHealingSessionServer::NUM_CLIENTS] IS_PERSISTENT;
static Ext2Inode    __inode_container[SelfHealingSessionServer::NUM_CLIENTS] IS_PERSISTENT;

Int
Ext2FsServer::AllocateFileContainer()
{
    for (Int i = 0; i < NUM_CLIENTS; i++) {
        if (__index[i] == 0) {
            __index[i] = 1;
            return i;
        }
    }
    return -1;
}

void
Ext2FsServer::ReleaseFileContainer(Int i)
{
    __index[i] = 0;
}

stat_t
Ext2FsServer::HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    L4_Word_t   mode;
    L4_Word_t   reg[3];
    addr_t      base;
    size_t      len;
    char*       path;
    SessionControlBlock* c;
    Ext2File*   file;
    Int         index;

    ENTER;

    if (Ipc::CheckPayload(&msg, 0, 2)) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_INVALID_ARGUMENTS);
        return ERR_NONE;
    }

    base = L4_Get(&msg, 0);
    c = Search(tid, base);

    mode = L4_Get(&msg, 1);

    len = strlen((const char *)c->base);
    path = (char *)malloc(len + 1);

    memcpy(path, (const void *)c->base, len + 1);

    file = _e2fs->Open(path, mode);
    if (file == 0) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    mfree(path);

    reg[0] = 0;
    reg[1] = file->Ino();
    reg[2] = file->Size();
    L4_Put(&msg, 0, 3, reg, 0, 0);

    index = AllocateFileContainer();
    // Deep copy
    __file_container[index].Copy(file);
    __inode_container[index] = *file->Inode();
    __file_container[index]._inode = &__inode_container[index];
    delete file;
    c->data = index;
    DOUT("persistent file object allocated: slot %d @ %p & %p\n",
         index, &__file_container[index], &__inode_container[index]);

    EXIT;
    return ERR_NONE;
}

stat_t
Ext2FsServer::HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    SessionControlBlock* c;
    addr_t      base;
    ENTER;

    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    if (c != 0 && c->data != -1UL) {
        DOUT("persistent file object released: slot %d @ %p\n",
             c->data, &__file_container[c->data]);
        __file_container[c->data].Flush();
        ReleaseFileContainer(c->data);
        c->data = -1UL;
    }

    L4_Clear(&msg);
    L4_Set_Label(&msg, ERR_NONE);
    EXIT;
    return ERR_NONE;
}

//static UInt counter = 0;

stat_t
Ext2FsServer::HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    size_t                  read;
    size_t                  length;
    L4_Word_t               offset;
    L4_Word_t               base;
    L4_Word_t               reg[2];
    SessionControlBlock*    c;
    Ext2File*               file;

    ENTER;

    //XXX: Fault injection
    /*
    counter++;
    if (counter % 5 == 0) {
        System.Print("!!! Ext2: FI %lu :-(\n", counter);
        *(L4_Word_t*)0 = 1;
    }
    */

    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    if (c == 0 || c->data == static_cast<word_t>(-1)) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    length = static_cast<size_t>(L4_Get(&msg, 1));
    offset = L4_Get(&msg, 2);

    file = &__file_container[c->data];
    file->Read(reinterpret_cast<void*>(c->base), length, offset, &read);
    DOUT("len %lu offset %lu read %lu\n", length, offset, read);

    reg[1] = read;
    L4_Put(&msg, 0, 2, reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

stat_t
Ext2FsServer::HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    size_t                  written;
    size_t                  length;
    L4_Word_t               offset;
    L4_Word_t               base;
    L4_Word_t               reg[2];
    SessionControlBlock*    c;
    Ext2File*               file;

    ENTER;

    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    if (c == 0 || c->data == static_cast<word_t>(-1)) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    length = (size_t)L4_Get(&msg, 2);
    offset = L4_Get(&msg, 3);

    file = &__file_container[c->data];
    file->Write(reinterpret_cast<const void*>(c->base), length, offset,
                &written);

    reg[1] = written;
    L4_Put(&msg, 0, 2, reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

const char* Ext2FsServer::DEFAULT_DISK_SERVER = "pata";

//SECTION(SEC_INIT)
stat_t
Ext2FsServer::Initialize0(Int argc, char* argv[])
{
    stat_t      err;
    const char* server;
    int         pn;
    ENTER;

    _disk = new Disk();
    if (_disk == 0) {
        FATAL("Ex2: Out of memory at init");
        return static_cast<stat_t>(-1);
    }

    if (argc > 2) {
        server = static_cast<const char*>(argv[1]);
        pn = atoi(argv[2]);
    }
    else if (argc > 1) {
        server = static_cast<const char*>(argv[1]);
        pn = DEFAULT_PARTITION;
    }
    else {
        server = DEFAULT_DISK_SERVER;
        pn = DEFAULT_PARTITION;
    }

    System.Print("'%s' looking for '%s'...\n", argv[0], server);
    err = _disk->Initialize(server, DEFAULT_PORT, DEFAULT_DISK);
    if (err != ERR_NONE) {
        return static_cast<stat_t>(-1);
    }
    System.Print("done %.8lX\n", _disk->Id().raw);

    if ((err = _disk->Open()) != ERR_NONE) {
        System.Print("%s\n", stat2msg[err]);
        FATAL("Ext2: disk open failed");
        return static_cast<stat_t>(-1);
    }

    System.Print("Open partition %d\n", pn);
    _partition = _disk->GetPartition(pn, Partition::LINUX);
    if (_partition == 0) {
        FATAL("Ext2: partition not found");
        return static_cast<stat_t>(-1);
    }

    _e2p = new Ext2Partition;
    if (_e2p->Initialize(_partition) != ERR_NONE) {
        System.Print("%s\n", stat2msg[err]);
        _disk->DumpPartitions();
        FATAL("Ext2: broken partition");
        goto exit;
    }

    _e2fs = new Ext2Fs(_e2p);
    if (_e2fs == 0) {
        delete _e2p;
        goto exit;
    }

    EXIT;
    return ERR_NONE;
exit:
    _disk->ReleasePartition(_partition);
    return static_cast<stat_t>(-1);
}

//SECTION(SEC_INIT)
stat_t
Ext2FsServer::Initialize(Int argc, char* argv[])
{
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ReleaseFileContainer(i);
    }
    return Initialize0(argc, argv);
}

stat_t
Ext2FsServer::Recover(Int argc, char* argv[])
{
    stat_t  err;
    
    err = Initialize0(argc, argv);
    if (err != ERR_NONE) {
        return err;
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (__index[i] != 0) {
            __file_container[i]._partition = _e2p;
            DOUT("Recover persistent file object %p inode %p\n",
                 &__file_container[i], __file_container[i]._inode);
        }
    }

    return ERR_NONE;
}

stat_t
Ext2FsServer::Exit()
{
    delete _e2fs;
    delete _e2p;
    _disk->ReleasePartition(_partition);
    delete _disk;

    return ERR_NONE;
}

ARC_SHS_SERVER(Ext2FsServer)

