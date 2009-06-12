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
/// @file   Services/Root/NameService.h
/// @author Hiroo Ishikawa <ishikawa@cs.waseda.jp>
/// @since  2007
///

// $Id: NameService.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_ROOT_NAME_SERVICE_H
#define ARC_ROOT_NAME_SERVICE_H

#include <List.h>
#include <Mutex.h>
#include <l4/types.h>

struct NameEntry
{
    char*           name;
    L4_ThreadId_t   tid;
};

class NameService
{
private:
    List<NameEntry*>    _list;
    Mutex               _mutex;

public:
    NameService() {};
    virtual ~NameService() {};
    virtual void Insert(const char *str, L4_ThreadId_t tid);
    virtual L4_ThreadId_t Search(const char *str);
    virtual void Remove(const char *str);
    virtual Iterator<NameEntry*>& GetList();
};

#endif // ARC_ROOT_NAME_SERVICE_H 

