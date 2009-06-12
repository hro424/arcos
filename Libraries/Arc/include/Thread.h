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
/// @brief  Simple thread library
/// @file   Include/arc/Thread.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: Thread.h 386 2008-08-30 05:07:57Z hro $

#ifndef ARC_THREAD_H
#define ARC_THREAD_H

#include <Debug.h>
#include <Ipc.h>
#include <List.h>
#include <Mutex.h>
#include <PageAllocator.h>
#include <Assert.h>
#include <Types.h>
#include <sys/Config.h>

template <size_t STACK_SIZE = PAGE_SIZE>
class Thread
{
public:
    ///
    /// The priority of a thread when it is created.
    ///
    static const UInt   DEFAULT_PRIORITY = 100;

    ///
    /// The highest priority
    ///
    static const UInt   HIGHEST_PRIORITY = 0xFF;

    ///
    /// The lowest priority
    ///
    static const UInt   LOWEST_PRIORITY = 0;

protected:
    ///
    /// Represents the ready state of a thread
    ///
    static const UByte  READY = 0;

    ///
    /// Represents the running state of a thread
    ///
    static const UByte  RUNNING = 1;

    ///
    /// Indicates that the thread is terminated and the object is going to be
    /// removed.
    ///
    static const UByte  TERMINATED = 0xFF;

    ///
    /// L4 thread ID
    ///
    L4_ThreadId_t       _tid;

    ///
    /// The entry point of the thread
    ///
    addr_t              _ip;

    ///
    /// The initial stack pointer of the thread
    ///
    addr_t              _sp;

    ///
    /// The current priority of the thread
    ///
    UInt                _priority;

    ///
    /// The current state of the thread
    ///
    UByte               _state;

    Mutex               _state_lock;

    ///
    /// List for thread join
    ///
    List<L4_ThreadId_t> _listeners;

    ///
    /// Mutex for the list for thread join
    ///
    Mutex               _join_lock;

    L4_ThreadId_t       _destoractor;

    ///
    /// The default entry point
    ///
    static void BootStrap(Thread<STACK_SIZE> *th);

    void SetState(UByte s);

    UByte GetState();

    ///
    /// Adds a joining thread
    ///
    void AddListener(L4_ThreadId_t tid);

    ///
    /// Removes a joining thread
    ///
    void DelListener(L4_ThreadId_t tid);

public:
    Thread(size_t stack_size);
    virtual ~Thread();

    ///
    /// Obtains the L4 thread ID of the thread
    ///
    L4_ThreadId_t Id() const;

    ///
    /// Defines the behavior of the thread
    ///
    virtual void Run() = 0;

    ///
    /// Activate the thread.  This method always start the thread at the entry
    /// point.
    ///
    stat_t Start();

    ///
    /// Blocks until the thread ends.
    ///
    void Join();

    ///
    /// Cancels current IPC
    ///
    stat_t Cancel();

    Bool IsRunning() const;

    UInt GetPriority() const;
    stat_t SetPriority(UInt prio);
};





template <size_t STACK_SIZE>
Thread<STACK_SIZE>::Thread(size_t stack_size = STACK_SIZE)
    : _priority(DEFAULT_PRIORITY), _state(READY), _destoractor(L4_nilthread)
{
    UInt        count;
    L4_Msg_t    msg;
    stat_t      err;

    L4_Put(&msg, MSG_ROOT_NEW_TH, 0, 0, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        System.Print(System.ERROR, "thread creation failed\n");
        return;
    }

    _tid.raw = L4_Get(&msg, 0);

    // Allocate stack
    for (count = 0; (1UL << count) < PAGE_ALIGN(stack_size); count++) ;
    _sp = palloc(count) + stack_size - sizeof(this);
    *reinterpret_cast<addr_t *>(_sp) = reinterpret_cast<addr_t>(this);
    _sp -= 4;

    //_priority = DEFAULT_PRIORITY;
    //_state = READY;
}

template <size_t STACK_SIZE>
Thread<STACK_SIZE>::~Thread()
{
    L4_Msg_t    msg;
    L4_Word_t   reg[2];
    stat_t      err;

    _state_lock.Lock();
    if (_state != TERMINATED) {
        _destoractor = L4_Myself();
        _state_lock.Unlock();
        L4_Receive(_tid);
    }
    else {
        _state_lock.Unlock();
    }

    reg[0] = _tid.raw;
    reg[1] = L4_Myself().raw;
    L4_Put(&msg, MSG_ROOT_DEL_TH, 2, reg, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        System.Print(System.ERROR, "thread deletion failed\n");
        return;
    }
}

template <size_t STACK_SIZE>
L4_ThreadId_t
Thread<STACK_SIZE>::Id() const
{
    return _tid;
}

template <size_t STACK_SIZE>
void
Thread<STACK_SIZE>::SetState(UByte s)
{
    _state_lock.Lock();
    _state = s;
    _state_lock.Unlock();
}

template <size_t STACK_SIZE>
UByte
Thread<STACK_SIZE>::GetState()
{
    UByte s;
    _state_lock.Lock();
    s = _state;
    _state_lock.Unlock();
    return s;
}

template <size_t STACK_SIZE>
void
Thread<STACK_SIZE>::AddListener(L4_ThreadId_t tid)
{
    _join_lock.Lock();
    _listeners.Add(tid);
    _join_lock.Unlock();
}

template <size_t STACK_SIZE>
void
Thread<STACK_SIZE>::DelListener(L4_ThreadId_t tid)
{
    _join_lock.Lock();
    _listeners.Remove(tid);
    _join_lock.Unlock();
}

template <size_t STACK_SIZE>
void
Thread<STACK_SIZE>::BootStrap(Thread<STACK_SIZE> *th)
{
    assert(th != 0);
    th->Run();
    th->SetState(READY);

    Iterator<L4_ThreadId_t>& it = th->_listeners.GetIterator();
    while (it.HasNext()) {
        L4_ThreadId_t tid = it.Next();
        L4_Send(tid);
        th->DelListener(tid);
    }
    th->SetState(TERMINATED);
    if (!L4_IsNilThread(th->_destoractor)) {
        L4_Send(th->_destoractor);
    }
    L4_Sleep(L4_Never);
}

template <size_t STACK_SIZE>
stat_t
Thread<STACK_SIZE>::Start()
{
    if (IsRunning()) {
        return ERR_NONE;
    }

    L4_Msg_t    msg;
    L4_Word_t   reg[3];
    stat_t      err;

    reg[0] = _tid.raw;
    reg[1] = (L4_Word_t)BootStrap;
    reg[2] = _sp;

    L4_Put(&msg, MSG_PEL_START_TH, 3, reg, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    SetState(RUNNING);

    return ERR_NONE;
}

template <size_t STACK_SIZE>
stat_t
Thread<STACK_SIZE>::Cancel()
{
    L4_Msg_t    msg;
    stat_t      err;

    L4_Put(&msg, MSG_PEL_CANCEL, 1, &_tid.raw, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }
    return ERR_NONE;
}

template <size_t STACK_SIZE>
void
Thread<STACK_SIZE>::Join()
{
    _state_lock.Lock();
    if (_state == RUNNING) {
        AddListener(L4_Myself());
        _state_lock.Unlock();
        L4_Receive(_tid);
    }
    else {
        _state_lock.Unlock();
    }
}

template <size_t STACK_SIZE>
Bool
Thread<STACK_SIZE>::IsRunning() const
{
    return _state == RUNNING;
}

template <size_t STACK_SIZE>
UInt
Thread<STACK_SIZE>::GetPriority() const
{
    return _priority;
}

template <size_t STACK_SIZE>
stat_t
Thread<STACK_SIZE>::SetPriority(UInt prio)
{
    L4_Word_t   reg[2];
    L4_Msg_t    msg;
    stat_t      err;

    if (prio > HIGHEST_PRIORITY) {
        return ERR_INVALID_ARGUMENTS;
    }

    reg[0] = _tid.raw;
    reg[1] = prio;
    L4_Put(&msg, MSG_ROOT_PRIO, 2, reg, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return err;
    }

    _priority = prio;

    return ERR_NONE;
}

#endif // ARC_THREAD_H
