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
/// @brief  Interrupt service library
/// @file   Include/arc/interrupt.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Interrupt.cpp 413 2008-09-09 01:38:42Z hro $

#include <Debug.h>
#include <Interrupt.h>
#include <Ipc.h>
#include <Thread.h>
#include <Types.h>


InterruptManager* InterruptManager::_self = 0;

stat_t
InterruptManager::Register(InterruptHandler* handler, UInt irq)
{
    ENTER;

    if (_iht_table[irq] == 0) {
        L4_Msg_t msg;
        L4_ThreadId_t tid;
        stat_t err;

        _iht_table[irq] = new InterruptHelper(handler);

        DOUT("new INTR helper: %.8lX\n", _iht_table[irq]->Id().raw);

        tid = L4_GlobalId(L4_ThreadNo(_iht_table[irq]->Id()), irq);
        L4_Put(&msg, MSG_ROOT_SET_INT, 1, &(tid.raw), 0, 0);
        err = Ipc::Call(L4_Pager(), &msg, &msg);
        if (err != ERR_NONE) {
            delete _iht_table[irq];
            return err;
        }

        _iht_table[irq]->Start();
    }

    EXIT;
    return ERR_NONE;
}

InterruptHandler *
InterruptManager::Deregister(UInt irq)
{
    InterruptHandler    *handler = 0;

    if (_iht_table[irq] != 0) {
        L4_Msg_t msg;
        stat_t err;

        L4_Put(&msg, MSG_ROOT_UNSET_INT, 1, (L4_Word_t *)&irq, 0, 0);
        err = Ipc::Call(L4_Pager(), &msg, &msg);
        if (err != ERR_NONE) {
            return 0;
        }

        handler = _iht_table[irq]->GetHandler();
        _iht_table[irq]->Cancel();
        delete _iht_table[irq];
        _iht_table[irq] = 0;
    }

    return handler;
}

