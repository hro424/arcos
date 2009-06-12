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
/// @file   Services/Devices/Parallel/Parallel.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  March 2008
/// 

//$Id: Parallel.cc 349 2008-05-29 01:54:02Z hro $

#include <Debug.h>
#include <Driver.h>
#include <Ipc.h>
#include <Parallel.h>
#include <System.h>
#include <Types.h>

#include <arc/IO.h>

#include <l4/ipc.h> // For L4_Sleep
#include <l4/types.h>

/* Masks for status port */
#define PR_ERROR (1 << 3)
#define SLCT (1 << 4)
#define PE (1 << 5)
#define ACK (1 << 6)
#define BUSY (1 << 7)

/* Masks for control port */
#define STROBE (1 << 0)
#define AUTOFD (1 << 1)
#define INIT (1 << 2)
#define SLCTIN (1 << 3)

/* Port addresses */
#define DATAPORT 0x378
#define STATUSPORT 0x379
#define CONTROLPORT 0x37a

static L4_Time_t SEND_DELAY;

class ParallelDriver : public DeviceDriver {
protected:
    stat_t Service(const L4_ThreadId_t& tid, L4_Msg_t& msg);
public:
    const char *const Name() { return "parallel"; }
    
    stat_t Initialize();
    stat_t Recover();
    stat_t Exit();
};

stat_t writeData(const char *const buffer, size_t count);

stat_t ParallelDriver::Initialize()
{
    SEND_DELAY = L4_TimePeriod(5);
    System.Print("Parallel driver started\n");
    return ERR_NONE;
}

#ifdef EVAL_RECOVERY_TIME
static ULong before IS_PERSISTENT;
#endif // EVAL_RECOVERY_TIME

stat_t ParallelDriver::Recover() {
#ifdef EVAL_RECOVERY_TIME
    ULong after = Rdtsc();
    if (after > before) {
        System.Print(System.INFO, "recovery time: %llu\n", after - before);
    }
#endif // EVAL_RECOVEY_TIME

    SEND_DELAY = L4_TimePeriod(5);
    // Make sure the port is in a neutral state,
    // by rising STROBE and waiting a little bit...
    Byte control = inb(CONTROLPORT);
    control |= STROBE; 
    outb(CONTROLPORT, control);
    L4_Sleep(SEND_DELAY);
    return ERR_NONE;
}

stat_t ParallelDriver::Exit() {
    return ERR_NONE;
    
}

static int * crashValue = 0;
static size_t i IS_PERSISTENT;

stat_t writeData(const char *const buffer, size_t count)
{
    Byte control = inb(CONTROLPORT);
    // Only start from scratch if this is a new request -
    // otherwise continue from the previous version of i
    if (!Recovered) {
        i = 0;
    }

    while (i < count) {
        // Put STROBE down to signal we send data
        control &= ~STROBE;
        outb(DATAPORT, buffer[i]);
        outb(CONTROLPORT, control);
        // Data is written to the port, we can consider
        // the character is processed
        i++;
        L4_Sleep(SEND_DELAY);
        // Rise STROBE and wait a little bit
        control |= STROBE; 
        outb(CONTROLPORT, control);
        L4_Sleep(SEND_DELAY);
        // Make a nice crash! :)
        if (i == 10 && !Recovered) {
#ifdef EVAL_RECOVERY_TIME
            before = Rdtsc();
#endif // EVAL_RECOVERY_TIME
            *crashValue = 5;
        }
        // If necessary, wait until the other end becomes
        // ready again
        Byte status = inb(STATUSPORT);
        while (status & BUSY) {
            L4_Sleep(SEND_DELAY);
            status = inb(STATUSPORT);
        }
        // Check for errors
        if (!(status & PR_ERROR)) {
            System.Print(System.ERROR, "parallel: error sending data\n");
            return ERR_UNKNOWN;
        }
    }
    return ERR_NONE;
}

static stat_t HandleWrite(const L4_ThreadId_t &tid, L4_Msg_t* msg)
{
    L4_Word_t count = L4_MsgWord(msg, 0);
    return writeData(reinterpret_cast<const char *>(&msg->raw[2]), count);
}

ARC_DRIVER(ParallelDriver)

BEGIN_HANDLERS(ParallelDriver);
    CONNECT_HANDLER(MSG_PARALLEL_WRITE, HandleWrite);
END_HANDLERS

