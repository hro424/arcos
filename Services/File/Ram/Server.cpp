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
/// @brief  RAM file system
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///

//$Id: Server.cpp 429 2008-11-01 02:24:02Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <MemoryAllocator.h>
#include <PageAllocator.h>
#include <MemoryManager.h>
#include <NameService.h>
#include <Server.h>
#include <Session.h>
#include <String.h>
#include <Types.h>
#include <l4/bootinfo.h>
#include <l4/kip.h>

class RamFsServer : public SessionServer
{
protected:
    addr_t  _ramfs_start;
    addr_t  _ramfs_end;
    size_t  _ramfs_size;

    virtual void Register(const L4_ThreadId_t& tid, addr_t base, size_t size);
    virtual stat_t HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg);

    stat_t FindBootRecord(const char *query, L4_Word_t type,
                                       L4_BootRec_t **record);
    virtual char* SearchFile(const char* name);
    virtual size_t GetFileSize(const char* file);
    virtual char* GetFilePointer(const char* file);

    void Dump();
public:
    virtual const char* const Name() { return "ram"; }
    virtual stat_t Initialize(Int argc, char* argv[]);
};

void
RamFsServer::Dump()
{
    char*   ptr = reinterpret_cast<char*>(_ramfs_start);
    while (ptr < reinterpret_cast<char*>(_ramfs_end)) {
#ifdef SYS_DEBUG
        char* name = ptr;
#endif // SYS_DEBUG
        while (*ptr != '\0') ptr++;
        ptr++;
        char* size = ptr;
        int to_next = atoi(size);
        DOUT("%s\t%s\t%d\n", name, size, to_next);
        while (*ptr != '\0') ptr++;
        ptr += to_next + 1;
        DOUT("next @ %p (%c)\n", ptr, *ptr);
    }
}

char*
RamFsServer::SearchFile(const char* name)
{
    char* ptr = reinterpret_cast<char*>(_ramfs_start);
    while (ptr < reinterpret_cast<char*>(_ramfs_end)) {
        if (strcmp(ptr, name) == 0) {
            return ptr;
        }
        else {
            while (*ptr != '\0') ptr++;
            ptr++;
            Int to_next = atoi(ptr);
            while (*ptr != '\0') ptr++;
            ptr += to_next + 1;
        }
    }
    return 0;
}

size_t
RamFsServer::GetFileSize(const char* file)
{
    char* ptr = const_cast<char*>(file);
    while (*ptr != '\0') ptr++;
    ptr++;
    return static_cast<size_t>(atoi(ptr));
}

char*
RamFsServer::GetFilePointer(const char* file)
{
    char* ptr = const_cast<char*>(file);
    while (*ptr != '\0') ptr++;
    ptr++;
    while (*ptr != '\0') ptr++;
    return ptr + 1;
}


struct RamClient : public SessionClient
{
    char*   file;
    RamClient(L4_ThreadId_t t, addr_t b, size_t s)
        : SessionClient(t, b, s), file(0) {}
};

void
RamFsServer::Register(const L4_ThreadId_t& tid, addr_t base, size_t size)
{
    RamClient* c = new RamClient(tid, base, size);
    if (c == 0) {
        FATAL("out of memory");
        return;
    }
    _clients.Append(c);
}


stat_t
RamFsServer::HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    addr_t      base;
    RamClient*  client;
    size_t      name_len;
    char*       name;
    L4_Word_t   reg[3];

    ENTER;

    base = L4_Get(&msg, 0);
    client = static_cast<RamClient*>(Search(tid, base));

    name_len = strlen(reinterpret_cast<char*>(client->base)) + 1;
    name = reinterpret_cast<char*>(malloc(name_len));
    memcpy(name, reinterpret_cast<char*>(client->base), name_len);

    if ((client->file = SearchFile(name)) == 0) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    DOUT("open '%s' size %d\n", name, GetFileSize(client->file));

    mfree(name);

    reg[0] = 0;
    reg[1] = 0;
    reg[2] = GetFileSize(client->file);
    L4_Put(&msg, 0, 3, reg, 0, 0);

    EXIT;
    return ERR_NONE;
}

stat_t
RamFsServer::HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    RamClient*  c;
    addr_t      base;
    ENTER;

    base = L4_Get(&msg, 0);
    c = static_cast<RamClient*>(Search(tid, base));
    if (c != 0 && c->file != 0) {
        c->file = 0;
    }

    L4_Clear(&msg);
    L4_Set_Label(&msg, ERR_NONE);

    EXIT;
    return ERR_NONE;
}

stat_t
RamFsServer::HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    addr_t      base;
    RamClient*  client;
    Int         file_len;
    Int         length;
    Int         offset;
    char*       ptr;
    L4_Word_t   reg[2];
    ENTER;

    base = L4_Get(&msg, 0);
    client = static_cast<RamClient*>(Search(tid, base));

    DOUT("tid %.8lX base %.8lX\n", tid.raw, base);
    if (client == 0 || client->file == 0) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    length = L4_Get(&msg, 1);
    offset = L4_Get(&msg, 2);

    file_len = GetFileSize(client->file);
    ptr = GetFilePointer(client->file);

    if (offset + length > file_len) {
        length = file_len - offset;
    }

    memcpy(reinterpret_cast<void*>(client->base), ptr + offset, length);

    reg[0] = 0;
    reg[1] = length;
    L4_Put(&msg, 0, 2, reg, 0, 0);

    EXIT;
    return ERR_NONE;
}


stat_t
RamFsServer::Initialize(Int argc, char* argv[])
{
    stat_t          err;
    L4_Word_t       rd_start;
    L4_Word_t       rd_size;
    L4_ThreadId_t   tid;

    err = NameService::Get("ramdisk_start", &tid);
    rd_start = tid.raw;
    NameService::Get("ramdisk_size", &tid);
    rd_size = tid.raw;
    DOUT("rd @%.8lX %lu bytes\n", rd_start, rd_size);

    _ramfs_start = palloc(rd_size / PAGE_SIZE + 1);
    _ramfs_size = rd_size;
    _ramfs_end = _ramfs_start + _ramfs_size;

    addr_t dest = _ramfs_start;
    addr_t source = rd_start;
    while (dest < _ramfs_end) {
        Pager.Map(dest, L4_ReadWriteOnly, source, L4_nilthread);
        dest += PAGE_SIZE;
        source += PAGE_SIZE;
    }

    Dump();

    return ERR_NONE;
}

ARC_SERVER(RamFsServer)

