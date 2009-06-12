/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @brief  Pager and loader
/// @file   Services/Pel2/Pel.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: Pel.h 410 2008-09-08 12:00:15Z hro $

#ifndef ARC_PEL2_PEL_H
#define ARC_PEL2_PEL_H

#include <LoadInfo.h>
#include <l4/types.h>
#include "IpcHandler.h"
#include "Loader.h"
#include "Task.h"

class Pel
{
protected:
    ///
    /// The counterpart of the corresponding user task
    ///
    Task            _task;

    ///
    /// The initializer of the Task object
    ///
    TaskLoader      _loader;

    ///
    /// The pager functionalities for the user task
    ///
    IpcHandler      _handler;

public:
    ///
    /// Obtains the id of the root task
    ///
    static L4_ThreadId_t RootTask();

    ///
    /// Obtains the information of the program to be loaded
    ///
    static const LoadInfo* GetLoadInfo();

    ///
    ///
    ///
    static L4_ThreadId_t GetFs(const char* fs);
};

#endif // ARC_PEL2_PEL_H

