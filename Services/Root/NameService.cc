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
/// @file   Services/Root/NameService.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: NameService.cc 429 2008-11-01 02:24:02Z hro $

#include <Debug.h>
#include <List.h>
#include <Mutex.h>
#include <String.h>
#include <l4/types.h>

#include "Common.h"
#include "NameService.h"

#include <System.h>


void
NameService::Insert(const char *str, L4_ThreadId_t tid)
{
    NameEntry *entry;

    entry = new NameEntry();

    entry->name = (char *)malloc(strlen(str) + 1);
    memcpy(entry->name, str, strlen(str) + 1);

    entry->tid = tid;

    _mutex.Lock();
    _list.Append(entry);
    _mutex.Unlock();
}

L4_ThreadId_t
NameService::Search(const char *str)
{
    L4_ThreadId_t           tid = L4_nilthread;
    Iterator<NameEntry*>&   it = _list.GetIterator();

    _mutex.Lock();
    while (it.HasNext()) {
        NameEntry* e = it.Next();
        if (strcmp(e->name, str) == 0) {
            tid = e->tid;
            break;
        }
    }
    _mutex.Unlock();
    return tid;
}

void
NameService::Remove(const char *str)
{
    Iterator<NameEntry*>&   it = _list.GetIterator();

    _mutex.Lock();
    while (it.HasNext()) {
        NameEntry* e = it.Next();
        if (strcmp(e->name, str) == 0) {
            _list.Remove(e);
            mfree(e->name);
            delete e;
            break;
        }
    }
    _mutex.Unlock();
}

Iterator<NameEntry*>&
NameService::GetList()
{
    return _list.GetIterator();
}

/*
void
NameService::AddListener(L4_ThreadId_t dest, const char* str)
{
    L4_TheadId_t    tid = Search(str);
    if (!L4_ThreadEqual(tid, L4_nilthread)) {
        Notify(dest, tid);
    }

    _listener_mutex.Lock();
    _listeners.Append(entry);
    _listener_mutex.Unlock();
}

void
NameService::RemoveListener(L4_ThreadId_t dest)
{
}

void
NameService::Notify(L4_ThreadId_t dest, L4_ThreadId_t tid)
{
}
*/
