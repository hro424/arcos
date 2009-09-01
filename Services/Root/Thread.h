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
/// @file   Services/Root/Thread.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

//$Id: Thread.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_ROOT_THREAD_H
#define ARC_ROOT_THREAD_H

#include <Mutex.h>
#include <List.h>
#include <l4/types.h>

class Space;

///
/// Models a thread - nothing special, just an aggregation of the
/// corresponding L4 elements.
///
struct Thread
{
    L4_ThreadId_t       Id;
    Space*              AddressSpace;
    L4_Word_t           Utcb;
    L4_Word_t           Irq;

    enum ThreadIdOffset {
        TID_S0_OFFSET =             0x00,   // Sigma0
        TID_S1_OFFSET =             0x01,   // Sigma1
        TID_ROOT_OFFSET =           0x02,   // Root task
        TID_PAGER_OFFSET =          0x03,   // Root pager
        TID_INIT_OFFSET =           0x04,   // Core service
        TID_OFFSET =                0x10,
    };

    static const L4_Word_t  TID_INITIAL_VERSION = 2;
};

#endif // ARC_ROOT_THREAD_H

