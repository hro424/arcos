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
/// @file   Services/Devices/Vesa/Vesa.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

#include <Debug.h>
#include <Ipc.h>
#include <System.h>
#include <String.h>
#include <SelfHealingServer.h>
#include <Session.h>
#include <MemoryManager.h>
#include <PersistentPageAllocator.h>

#include "Vbe.h"

static UInt __current_vmode IS_PERSISTENT;


class VbeServer : public SelfHealingSessionServer
{
protected:
    Vbe     _vbe;

    virtual stat_t HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg);

public:
    virtual const char* const Name() { return "vesa"; }

    virtual stat_t Initialize(Int argc, char* argv[])
    {
        ENTER;
        if (_vbe.Initialize() != ERR_NONE) {
            return ERR_FATAL;
        }

        _vbe.Print();

        EXIT;
        return ERR_NONE;
    }

    virtual stat_t Recover(Int argc, char* argv[])
    { 
        if (_vbe.Initialize() != ERR_NONE) {
            return ERR_FATAL;
        }
        return ERR_NONE;
    }

    virtual stat_t Exit()
    {
        _vbe.Finalize();
        System.Print("VBE driver exiting...\n");
        return ERR_NONE;
    }
};



stat_t
VbeServer::HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    ENTER;
    L4_Word_t   reg[5];
    addr_t      shm = shmalloc(Session::DEFAULT_SHM_PAGES);

    if (shm == 0) {
        return ERR_OUT_OF_MEMORY;
    }

    //TODO: Hack: Related to a problem in PersistentPageAllocator.cpp
    for (UInt i = 0; i < Session::DEFAULT_SHM_PAGES; i++) {
        Pager.Release(shm + i * PAGE_SIZE);
    }

    for (UInt i = 0; i < Session::DEFAULT_SHM_PAGES; i++) {
        Pager.Reserve(shm + i * PAGE_SIZE, tid, L4_ReadWriteOnly);
    }

    reg[0] = shm;
    reg[1] = Session::DEFAULT_SHM_PAGES;
    Register(tid, reg[0], reg[1]);

    // Return the supported video modes
    L4_Word_t nmode = _vbe.GetNumberOfVideoModes();
    for (L4_Word_t i = 0; i < nmode; i++) {
        const VideoMode* vobj = _vbe.GetVideoMode(i);
        memcpy((void*)(shm + sizeof(VideoMode) * i), vobj, sizeof(VideoMode));
    }
    reg[2] = _vbe.Info()->vbe_version;
    reg[3] = _vbe.Info()->GetTotalMemory();
    reg[4] = nmode;

    L4_Put(&msg, 0, 5, reg, 0, 0);
    EXIT;
    return ERR_NONE;
}

stat_t
VbeServer::HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    ENTER;
    SessionControlBlock*    c;
    L4_Word_t               base;
    L4_Word_t               mode;

    base = L4_Get(&msg, 0);
    c = Search(tid, base);
    if (c == 0) {
        L4_Clear(&msg);
        L4_Set_Label(&msg, ERR_NOT_FOUND);
        return ERR_NONE;
    }

    mode = L4_Get(&msg, 1);

    if (_vbe.SetVideoMode(mode) != ERR_NONE) {
        System.Print("Cannot set video mode!\n");
        return ERR_UNKNOWN;
    }

    __current_vmode = mode;

    L4_Clear(&msg);
    EXIT;
    return ERR_NONE;
}

ARC_SHS_SERVER(VbeServer)

