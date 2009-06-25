/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
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
/// @file   Services/Devices/Disk/Pata/Server.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Server.cc 426 2008-10-01 09:38:07Z hro $

#include <Debug.h>
#include <Ipc.h>
#include <MemoryAllocator.h>
#include <Mutex.h>
#include <SelfHealingServer.h>
#include <Session.h>
#include <String.h>
#include <Types.h>
#include "Ata.h"
#include "AtaCommandBlock.h"
#include "Port.h"

#include <l4/ipc.h>
#include <l4/types.h>
#include <l4/message.h>
#include <l4/schedule.h>

#include <System.h>

class PataServer : public SelfHealingSessionServer
{
protected:
    Port*       _port;

    virtual stat_t HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg);
public:
    virtual const char* const Name() { return "pata"; }

    virtual stat_t Initialize(Int argc, char* argv[]);

    virtual stat_t Recover(Int argc, char* argv[])
    { return Initialize(argc, argv); }

    virtual stat_t Exit()
    {
        ENTER;
        _port->DisableInterrupt();
        _port->DisableDMA();
        EXIT;
        return ERR_NONE;
    }
};

//FI: static UInt counter = 0;

stat_t
PataServer::HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    AtaCommandBlock cb;
    UInt            device;
    UInt            sectors;
    UInt            pos;
    addr_t          base;
    ENTER;

    if (Ipc::CheckPayload(&msg, 0, 4)) {
        L4_Put(&msg, ERR_INVALID_ARGUMENTS, 0, 0, 0, 0);
        return ERR_NONE;
    }

    //XXX: Fault injection
    //FI: counter++;
    /*
    if (counter % 1400 == 0) {
        System.Print("!!! Pata: FI %lu :-(\n", counter);
        System.Exit(2);
    }
    */

    base = static_cast<addr_t>(L4_Get(&msg, 0));
    device = L4_Get(&msg, 1);
    device &= 0xFFFF;

    if (!(0 <= device && device < 4)) {
        L4_Put(&msg, ERR_NOT_FOUND, 0, 0, 0, 0);
        return ERR_NONE;
    }

    // Read data from the device and write it into the shared space
    pos = L4_Get(&msg, 2);
    sectors = L4_Get(&msg, 3);

    // Setup ATA command block
    cb.Initialize();
    cb.count = sectors;
    cb.SetLba(pos);
    cb.command = ATA_CMD_READ_MULTI;
    //cb.command = ATA_CMD_READ_DMA;
    //cb.command = ATA_CMD_IDENTIFY_DMA;

    if (device % 2 == 0) {
        cb.Device0();
    }
    else {
        cb.Device1();
    }

    UByte err = _port->DataIn(&cb, base);
    //UByte err = _port->DMATransfer(&cb, base);
    if (err != ERR_NONE) {
        DOUT("ERROR (0x%.2X)\n", err);
        //return ERR_NONE;
    }

    /*
    unsigned int* ptr = (unsigned int*)base;
    for (int i= 0; i < 128; i += 4) {
        DOUT("%x %x %x %x\n", ptr[i], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
    }
    */
    EXIT;
    return Ipc::ReturnError(&msg, ERR_NONE);
}

stat_t
PataServer::HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg)
{
    AtaCommandBlock cb;
    UInt            device;
    UInt            sectors;
    UInt            loc;
    addr_t          base;

    ENTER;

    if (Ipc::CheckPayload(&msg, 0, 4)) {
        L4_Put(&msg, ERR_INVALID_ARGUMENTS, 0, 0, 0, 0);
        return ERR_NONE;
    }

    base = static_cast<addr_t>(L4_Get(&msg, 0));
    device = L4_Get(&msg, 1);
    device &= 0xFFFF;

    if (!(0 <= device && device < 4)) {
        L4_Put(&msg, ERR_NOT_FOUND, 0, 0, 0, 0);
        return ERR_NONE;
    }

    // Read data from the device and write it into the shared space
    loc = L4_Get(&msg, 2);
    sectors = L4_Get(&msg, 3);

    cb.Initialize();
    cb.count = sectors;
    cb.SetLba(loc);
    cb.command = ATA_CMD_WRITE_MULTI;

    if (device % 2 == 0) {
        cb.Device0();
    }
    else {
        cb.Device1();
    }

    _port->DataOut(&cb, reinterpret_cast<const void*>(base));

    EXIT;
    return Ipc::ReturnError(&msg, ERR_NONE);
}

stat_t
PataServer::Initialize(Int argc, char* argv[])
{
    ENTER;
    switch (argc) {
        case 1:
            _port = new Port(0);
            break;
        case 2:
            if (strncmp(argv[1], "0", 2) == 0) {
                _port = new Port(0);
            }
            else if (strncmp(argv[1], "1", 2) == 0) {
                _port = new Port(1);
            }
            else {
                return ERR_INVALID_ARGUMENTS;
            }
            break;
        default:
            return ERR_INVALID_ARGUMENTS;
    }

    _port->EnableDMA();
    _port->EnableInterrupt();

    AtaCommandBlock cb;
    UShort          buf[256];

    cb.Initialize();
    cb.count = 1;
    cb.command = ATA_CMD_IDENTIFY;
    _port->DataIn(&cb, (addr_t)buf);

    System.Print("C:H:S %u:%u:%u\n", buf[1], buf[3], buf[6]);

    char* p = (char*)&buf[27];
    for (int i = 0; i < 20; i += 2) {
        System.Print("%c%c", p[i + 1], p[i]);
    }
    System.Print(" Rev.");
    p = (char*)&buf[23];
    for (int i = 0; i < 8; i += 2) {
        System.Print("%c%c", p[i + 1], p[i]);
    }
    System.Print(" S/N ");
    p = (char*)&buf[10];
    for (int i = 0; i < 10; i += 2) {
        System.Print("%c%c", p[i + 1], p[i]);
    }
    System.Print("\n");

    System.Print("DMA Mode Supported: %s%s%s%s%s%s%s%s\n",
                 buf[63] & 0x1 ? "0" : "",
                 (buf[63] >> 1) & 0x1 ? ",1" : "",
                 (buf[63] >> 2) & 0x1 ? ",2" : "",
                 (buf[63] >> 3) & 0x1 ? ",3" : "",
                 (buf[63] >> 4) & 0x1 ? ",4" : "",
                 (buf[63] >> 5) & 0x1 ? ",5" : "",
                 (buf[63] >> 6) & 0x1 ? ",6" : "",
                 (buf[63] >> 7) & 0x1 ? ",7" : "");

    System.Print("DMA Mode Active: %s%s%s%s%s%s%s%s\n",
                 (buf[63] >> 8) & 0x1 ? "0" : "",
                 (buf[63] >> 9) & 0x1 ? ",1" : "",
                 (buf[63] >> 10) & 0x1 ? ",2" : "",
                 (buf[63] >> 11) & 0x1 ? ",3" : "",
                 (buf[63] >> 12) & 0x1 ? ",4" : "",
                 (buf[63] >> 13) & 0x1 ? ",5" : "",
                 (buf[63] >> 14) & 0x1 ? ",6" : "",
                 (buf[63] >> 15) & 0x1 ? ",7" : "");

    System.Print("ATA Version: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
                 buf[80] & 0x1 ? "1" : "",
                 (buf[80] >> 1) & 0x1 ? ",2" : "",
                 (buf[80] >> 2) & 0x1 ? ",3" : "",
                 (buf[80] >> 3) & 0x1 ? ",4" : "",
                 (buf[80] >> 4) & 0x1 ? ",5" : "",
                 (buf[80] >> 5) & 0x1 ? ",6" : "",
                 (buf[80] >> 6) & 0x1 ? ",7" : "",
                 (buf[80] >> 7) & 0x1 ? ",8" : "",
                 (buf[80] >> 8) & 0x1 ? ",9" : "",
                 (buf[80] >> 9) & 0x1 ? ",10" : "",
                 (buf[80] >> 10) & 0x1 ? ",11" : "",
                 (buf[80] >> 11) & 0x1 ? ",12" : "",
                 (buf[80] >> 12) & 0x1 ? ",13" : "",
                 (buf[80] >> 13) & 0x1 ? ",14" : "",
                 (buf[80] >> 14) & 0x1 ? ",15" : "");

    EXIT;
    return ERR_NONE;
}

ARC_SHS_SERVER(PataServer)

