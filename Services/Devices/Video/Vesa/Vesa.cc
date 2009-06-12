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

//$Id: Vesa.cc 349 2008-05-29 01:54:02Z hro $


#include "ServerScreen.h"
#include "ServerVideoMode.h"

#include <Debug.h>
#include <Driver.h>
#include <Ipc.h>
#include <System.h>
#include <Types.h>
#include <sys/Config.h>
#include <vesa/protocol.h>
#include <l4/types.h>

static stat_t HandleScreenSize(const L4_Thread_id &tid, L4_Msg_t *msg);
static stat_t HandleInit(const L4_Thread_id &tid, L4_Msg_t *msg);
static stat_t HandleSetVideoMode(const L4_Thread_id &tid, L4_Msg_t *msg);

static char persistentarea[PAGE_SIZE * 2] __attribute__ ((aligned (PAGE_SIZE))) IS_PERSISTENT;
static ServerScreen *serverScreen IS_PERSISTENT;

#define SCREEN_DESC_SIZE                        \
    (sizeof(ServerScreen) + sizeof(VideoMode) * \
     (serverScreen->GetNbSupportedModes()))

#define SCREEN_DESC_SIZE_IN_PAGES   \
    ((SCREEN_DESC_SIZE + PAGE_SIZE - 1) / PAGE_SIZE)

class VesaDriver : public DeviceDriver {
public:
    const char *const Name() { return "vesa"; }

    stat_t Initialize();
    stat_t Recover();
    stat_t Exit();
    stat_t Service(const L4_ThreadId_t& tid, L4_Msg_t& msg);
};


stat_t
VesaDriver::Initialize()
{
    System.Print(System.INFO, "VBE driver starting...\n");

    // First get the page for the screen
    // How many pages do we need?
    /* int nbPages = 2;
       serverScreen = (ServerScreen *)palloc(nbPages); */

    for (UInt i = 0; i < PAGE_SIZE * 2; i++) {
        persistentarea[i] = 0;
    }

    serverScreen = (ServerScreen *)persistentarea;
    if (serverScreen->Initialize() != ERR_NONE) {
        System.Print(System.ERROR, "vesa: Cannot initialize screen!\n");
        return ERR_NOT_FOUND;
    }

    return ERR_NONE;
}

stat_t
VesaDriver::Recover()
{
    return ERR_NONE;
}

stat_t
VesaDriver::Exit()
{
    // TODO: Enable that when x86emu unmaps the bios pages on cleanup.
    // pfree(main_mem);
    serverScreen->CleanUp();
    System.Print(System.INFO, "VBE driver exiting...\n");
    return ERR_NONE;
}

static stat_t
HandleScreenSize(const L4_Thread_id &tid, L4_Msg_t *msg)
{
    // Return how many pages we need to keep the screen data structures
    L4_Word_t count = SCREEN_DESC_SIZE_IN_PAGES;
    L4_Put(msg, 0, 1, &count, 0, 0);
    return ERR_NONE;   
}

static stat_t
HandleInit(const L4_Thread_id &tid, L4_Msg_t *msg)
{
    // Map the pages containing the screen and video mode definitions
    L4_Word_t       dest;

    ENTER;
    DOUT("serverScreen @ %p\n", serverScreen);
    dest = L4_Get(msg, 0) & PAGE_MASK;
    L4_Clear(msg);
    for (UInt i = 0; i < SCREEN_DESC_SIZE_IN_PAGES; i++) {
        L4_Fpage_t fpage = L4_FpageLog2((L4_Word_t)serverScreen + (PAGE_SIZE * i),
                                        PAGE_BITS);
        L4_Set_Rights(&fpage, L4_FullyAccessible);
        L4_MapItem_t item = L4_MapItem(fpage, dest + (PAGE_SIZE * i));
        L4_MsgAppendMapItem(msg, item);
    }
    EXIT;
    return ERR_NONE;   
}

static stat_t
HandleSetVideoMode(const L4_Thread_id &tid, L4_Msg_t *msg)
{
    const ServerVideoMode*  mode;
    L4_MapItem_t    item;
    L4_Word_t       base;
    UInt            index;
    ENTER;

    index = L4_Get(msg, 0);
    serverScreen->SetVideoMode(index);
    mode = static_cast<const ServerVideoMode*>(
                                        serverScreen->GetSupportedModes());
    base = reinterpret_cast<L4_Word_t>(serverScreen->GetFrameBuffer());
    //FIXME: quick hack
    item = L4_MapItem(L4_FpageAddRights(L4_Fpage(base, mode->LFBSize()),
                                        L4_FullyAccessible),
                      0x80000000);
    L4_Put(msg, 0, 0, 0, 2, &item);
    EXIT;
    return ERR_NONE;
    /*
    const ServerVideoMode*   mode;
    L4_Word_t   reg[2];
    UInt        index;
   
    ENTER;

    index = L4_Get(msg, 0);

    // First set the requested mode...
    serverScreen->SetVideoMode(index);

    // Now map the linear frame buffer
    mode = static_cast<const ServerVideoMode*>(
                                        serverScreen->GetSupportedModes());
    reg[0] = reinterpret_cast<addr_t>(serverScreen->GetFrameBuffer());
    reg[1]= mode[index].LFBSize();

    L4_Put(msg, ERR_NONE, 2, reg, 0, 0);

    EXIT;
    return ERR_NONE;
    */
}

ARC_DRIVER(VesaDriver);

BEGIN_HANDLERS(VesaDriver)
    CONNECT_HANDLER(MSG_VESA_SCREENSIZE, HandleScreenSize)
    CONNECT_HANDLER(MSG_VESA_INIT, HandleInit)
    CONNECT_HANDLER(MSG_VESA_SET_VIDEO_MODE, HandleSetVideoMode)
END_HANDLERS

