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
/// @brief  Define helper functions to use the naming service.
/// @file   Libraries/Arc/NameService.cpp
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
///

#include <Ipc.h>
#include <NameService.h>
#include <String.h>
#include <Types.h>

L4_ThreadId_t NameService::root_ns = { 0 };

// Maximum number of registers used for this message. This limits the length
// of names to (MAX_REGS - 1) * sizeof(L4_Word_t)
static const int MAX_REGS = 16;
static L4_Word_t regs[MAX_REGS + 2];
/// Returns the number of registers used by the string
#define STRING_NB_REGS(NAME) (strlen(NAME) + sizeof(L4_Word_t)) / sizeof(L4_Word_t)
/// Checks if the string fits into our registers. We save one register for storing the
/// tid.
#define STRING_LENGTH_OK(NAME) (STRING_NB_REGS(name) <= (MAX_REGS + 2) - 1)

stat_t NameService::Insert(const char *const name, const L4_ThreadId_t tid)
{
    L4_Msg_t msg;

    // First check that the string is not too long for the message
    if (!STRING_LENGTH_OK(name)) return ERR_OUT_OF_MEMORY;
    // Copy the tid and string into the registers
    regs[0] = tid.raw;
    strcpy((char *)&regs[1], name);
    
    // Number of regs transmitted is number of registers used by the string +
    // the register used to transfer the tid
    L4_MsgPut(&msg, MSG_NS_INSERT, STRING_NB_REGS(name) + 1, regs, 0, 0);
    return Ipc::Call(rootNS(), &msg, &msg);
}

stat_t NameService::Get(const char *const name, L4_ThreadId_t *const tid)
{
    L4_Msg_t msg;
    stat_t err;
    
    // First check that the string is not too long for the message
    if (!STRING_LENGTH_OK(name)) return ERR_OUT_OF_MEMORY;
    
    // Copy the string into the registers
    strcpy((char *)regs, name);
    
    L4_MsgPut(&msg, MSG_NS_SEARCH, STRING_NB_REGS(name), regs, 0, 0);
    err = Ipc::Call(rootNS(), &msg, &msg);
    if (err != ERR_NONE) return err;
    tid->raw = L4_MsgWord(&msg, 0);
    return err;
}


stat_t NameService::Remove(const char *const name)
{
   L4_Msg_t msg;
   
    // First check that the string is not too long for the message
    if (!STRING_LENGTH_OK(name)) return ERR_OUT_OF_MEMORY;
    
    // Copy the string into the registers
    strcpy((char *)regs, name);
    
    L4_MsgPut(&msg, MSG_NS_REMOVE, STRING_NB_REGS(name), regs, 0, 0);
    return Ipc::Call(rootNS(), &msg, &msg);
}

/*
stat_t
NameService::List()
{
    L4_Msg_t msg;
    L4_Put(&msg, MSG_NS_LIST, 0, 0, 0, 0);
    return Ipc::Call(rootNS(), &msg, &msg);
}
*/

stat_t
NameService::Enumerate(UInt index, NameEntry* entry)
{
    L4_Word_t   reg[MAX_REGS + 2];
    stat_t      err;
    L4_Msg_t    msg;
    L4_Put(&msg, MSG_NS_LIST, 1, &index, 0, 0);
    err = Ipc::Call(rootNS(), &msg, &msg);
    if (err == ERR_NONE && entry != 0) {
        L4_Get(&msg, reg, 0);
        entry->tid.raw = reg[0];
        entry->pager.raw = reg[1];
        strncpy(entry->name, (const char*)&reg[2], MAX_REGS * 4);
    }
    return err;
}

