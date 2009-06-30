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
/// @since  2008
/// 

//$Id: $
#include <stdio.h>
#include <arc/console.h>
#include <arc/status.h>

#include "ServerScreen.h"
#include "ServerVideoMode.h"
#include <vesa/protocol.h>

#include <arc/ipc.h>
#include <arc/memory.h>
#include <arc/thread.h>


static status_t HandleScreenSize(L4_Msg_t *msg);
static status_t HandleInit(L4_Msg_t *msg);
static status_t HandleSetVideoMode(L4_Msg_t *msg);

static ServerScreen *serverScreen;

#define SCREEN_DESC_SIZE (sizeof(ServerScreen) + sizeof(VideoMode) * (serverScreen->getNbSupportedModes()))
#define SCREEN_DESC_SIZE_IN_PAGES ((SCREEN_DESC_SIZE + PAGE_SIZE - 1) / PAGE_SIZE)

int
main(void)
{
    ConsoleOut(INFO, "VBE driver starting...\n");
    
    // First get the page for the screen
    // How many pages do we need?
    int nbPages = 2;
    serverScreen = (ServerScreen *)palloc(nbPages);
    
    if (serverScreen->init() != ERR_NONE) {
        ConsoleOut(ERROR, "vesa: Cannot initialize screen!\n");
        return ERR_NOT_FOUND;
    }
    screen = serverScreen;
    
    //screen->printInfo();
    
    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    status_t err;
    
    tag = L4_Wait(&tid);
    
    for (;;) {
        L4_MsgStore(tag, &msg);
        switch (L4_Label(tag)) {
        case MSG_VESA_SCREENSIZE:
            if ((err = HandleScreenSize(&msg)) != ERR_NONE)
                ConsoleOut(ERROR, "vesa:GET_SCREENSIZE: %s\n", stat2msg[err]);
            break;
        case MSG_VESA_INIT:
            if ((err = HandleInit(&msg)) != ERR_NONE)
                ConsoleOut(ERROR, "vesa:INIT: %s\n", stat2msg[err]);
            break;
        case MSG_VESA_SET_VIDEO_MODE:
            if ((err = HandleSetVideoMode(&msg)) != ERR_NONE)
                ConsoleOut(ERROR, "vesa:SET_VIDEO_MODE: %s\n", stat2msg[err]);
            break;
        default:
            ConsoleOut(WARN, "vesa: Unknown message %lX from %.8lX\n",
                       L4_Label(tag), tid.raw);
            break;
            
        }
        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }
    
    // TODO: Enable that when x86emu unmaps the bios pages on cleanup.
    // pfree(main_mem);
    serverScreen->cleanup();
	ConsoleOut(INFO, "VBE driver exiting...\n");
	return 0;
}

static status_t HandleScreenSize(L4_Msg_t *msg) {
    // Return how many pages we need to keep the screen data structures
    L4_Word_t nbPages = SCREEN_DESC_SIZE_IN_PAGES;
    L4_MsgPut(msg, 0, 1, &nbPages, 0, 0);
    return ERR_NONE;   
}

static status_t HandleInit(L4_Msg_t *msg) {
    // Map the pages containing the screen and video mode definitions
    L4_Word_t destAddr = L4_MsgWord(msg, 0);
    L4_MsgPut(msg, 0, 0, 0, 0, 0);
    for (unsigned int i = 0; i < SCREEN_DESC_SIZE_IN_PAGES; i++) {
        L4_MapItem_t mapItem = 
            L4_MapItem(L4_FpageLog2((L4_Word_t)screen + (PAGE_SIZE * i), PAGE_BITS),
                    (destAddr + (PAGE_SIZE * i)) & PAGE_MASK);
        L4_MsgAppendMapItem(msg, mapItem);
    }
    return ERR_NONE;   
}

static status_t HandleSetVideoMode(L4_Msg_t *msg) {
    // First set the requested mode...
    L4_Word_t modeIndex = L4_MsgWord(msg, 0);
    serverScreen->setVideoMode(modeIndex);
    
    // Now map the linear frame buffer
    const ServerVideoMode &mode = ((const ServerVideoMode *)serverScreen->getSupportedModes())[modeIndex];
    L4_Word_t lfbaddress = (L4_Word_t) screen->getFrameBuffer();
    L4_MapItem_t mapItem =
        L4_MapItem(L4_FpageAddRights(L4_Fpage(lfbaddress, mode.LFBSize()), L4_FullyAccessible),
                VBE_LFB_ADDRESS & PAGE_MASK);
    L4_MsgPut(msg, 0, 0, 0, 2, &mapItem);
    
    return ERR_NONE;
}
