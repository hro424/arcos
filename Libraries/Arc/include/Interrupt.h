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
/// @file   Include/arc/Interrupt.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

#ifndef ARC_INTERRUPT_H
#define ARC_INTERRUPT_H

#include <Thread.h>
#include <Assert.h>
#include <Types.h>


class InterruptHelper;

class InterruptHandler
{
private:
    L4_ThreadId_t   _tid;

public:
    virtual void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t *msg) = 0;

    L4_ThreadId_t Id() { return _tid; }

    friend class InterruptHelper;
};


class InterruptHelper : public Thread<>
{
private:
    InterruptHandler*   _handler;

    InterruptHelper();

public:
    InterruptHelper(InterruptHandler* handler) : Thread<>(), _handler(handler)
    { _handler->_tid = Id(); }

    virtual ~InterruptHelper() { _handler->_tid = L4_nilthread; }

    void Run()
    {
        ENTER;
        L4_Msg_t        msg;
        L4_MsgTag_t     tag;
        L4_ThreadId_t   tid;

        tag = L4_Wait(&tid);
        if (L4_IpcFailed(tag)) {
            return;
        }

        for (;;) {
            L4_Store(tag, &msg);

            assert(_handler != 0);
            _handler->HandleInterrupt(tid, &msg);

            L4_Clear(&msg);
            L4_Load(&msg);
            tag = L4_ReplyWait(tid, &tid);
            if (L4_IpcFailed(tag)) {
                break;
            }
        }
        EXIT;
    }

    InterruptHandler* GetHandler() const { return _handler; };
};


class InterruptManager
{
private:

    static const UInt           NIRQ = 16;
    static const UInt           PRIO_IRQ = 120;
    static InterruptManager*    _self;

    ///
    /// Table of the interrupt handling threads
    ///
    InterruptHelper*            _iht_table[NIRQ];

    InterruptManager()
    {
        for (UInt i = 0; i < NIRQ; i++) {
            _iht_table[i] = 0;
        }
    }

public:
    ///
    /// Returns the instance of InterruptManager.  InterruptManager is a
    /// singleton class.
    ///
    static InterruptManager* Instance()
    {
        if (_self == 0) {
            _self = new InterruptManager;
        }
        return _self;
    }

    ///
    /// Registers an interrupt handler to the specified IRQ.
    ///
    /// @param handler      the interrupt handler
    /// @param irq          the interrupt request number
    ///
    stat_t Register(InterruptHandler *handler, UInt irq)
    {
        ENTER;

        if (_iht_table[irq] == 0) {
            L4_Msg_t msg;
            L4_ThreadId_t tid;
            stat_t err;

            _iht_table[irq] = new InterruptHelper(handler);

            tid = L4_GlobalId(L4_ThreadNo(_iht_table[irq]->Id()), irq);
            L4_Put(&msg, MSG_ROOT_SET_INT, 1, &(tid.raw), 0, 0);
            err = Ipc::Call(L4_Pager(), &msg, &msg);
            if (err != ERR_NONE) {
                delete _iht_table[irq];
                return err;
            }

            _iht_table[irq]->SetPriority(PRIO_IRQ);
            err = _iht_table[irq]->Start();
        }

        EXIT;
        return ERR_NONE;
    }

    ///
    /// Deregisters the interrupt handler of the specified IRQ.
    ///
    /// @param irq          the interrupt request number
    /// @return             the interrupt handler that was registered
    ///
    InterruptHandler *Deregister(UInt irq)
    {
        ENTER;
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

        EXIT;
        return handler;
    }

};

#endif // ARC_INTERRUPT_H

