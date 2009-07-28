/*
 *
 *  Copyright (C) 2007, 2008, Waseda University. All rights reserved.
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
/// @brief  Provides the operations of address spaces and threads.
/// @file   Services/Root/Task.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: Task.h 386 2008-08-30 05:07:57Z hro $

#ifndef ARC_ROOT_TASK_H
#define ARC_ROOT_TASK_H

#include <Types.h>

#include <l4/types.h>
#include <l4/bootinfo.h>

class Pager;
struct Space;
struct Thread;

///
/// Initializes the task management
///
void InitializeTaskManagement();

///
/// Allocates and initializes an address space and create its root thread.
///
/// @param space        the new address space
///
stat_t CreateTask(Space **space, Bool shadow);

///
/// Deletes the address space object.
///
/// @param space        the space object
///
stat_t DeleteTask(Space *space);

///
/// Finds the address space object to which the specified thread is
/// running.
///
/// @param tid      the thread
/// @param space    the address space object where the thread blongs
///
stat_t FindTask(L4_ThreadId_t tid, Space **space);

stat_t ActivateThread(const Thread* thread, L4_ThreadId_t sched,
                      L4_ThreadId_t pager);

///
/// Creates a new inactive thread in the specified address space. A global
/// thread ID is automatically assigned to it.
///
/// @param space        the space where the thread is created
/// @param thread       the ID of the thread to be assigned
///
stat_t CreateThread(Space *space, Thread **thread);

///
/// Teminates the thread and deletes the thread object.
///
/// @param thread       the thread object
///
stat_t DeleteThread(Thread *thread);

///
/// Allocates a page for the stack in the space.  Note, the stack is placed
/// on the fixed virtual address.
///
/// @param s            the address space
/// @param phys         the physical address of the stack page
///
stat_t AllocateStack(Space *s, L4_Word_t *phys);

///
/// Releases the page allocated by TO_AllocateStack.
///
/// @param s            the address space
/// @param phys         the physical address of the page
///
void ReleaseStack(Space *s, L4_Word_t phys);

void RegisterInitialExecutable(Space *s, L4_BootRec_t *rec);
L4_Word_t RegisterInitialModule(Space* s, L4_BootRec_t* rec);

stat_t ExecInitialTask(L4_BootRec_t *task);
stat_t ExecInitialTaskPm(L4_BootRec_t *task);
stat_t ExecTask(L4_Word_t argc, char** argv, L4_ThreadId_t *tid);

///
/// Activates the specified thread with the specified instruction and stack
/// pointers. UTCB for the thread is automatically assigned.
///
/// @param thread       the thread
/// @param pager        the pager for the thread
/// @param ip           the instruction pointer
/// @param sp           the stack pointer
///
stat_t StartThread(Thread *th, L4_ThreadId_t pager, L4_Word_t ip, L4_Word_t sp);

#endif  // ARC_ROOT_TASK_H

