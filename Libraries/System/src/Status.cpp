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
/// @file   Libraries/System/Status.cpp
/// @since  November 2007
///

//$Id: Status.cpp 349 2008-05-29 01:54:02Z hro $

#include <Types.h>

const char* const stat2msg[NSTATUS] = {
    "",
    "Invalid rights (kernel)",
    "Invalid thread (kernel)",
    "Invalid space (kernel)",
    "Invalid schedule (kernel)",
    "Invalid arguments (kernel)",
    "Invalid UTCB (kernel)",
    "Invalid KIP (kernel)",
    "Out of memory (kernel)",
    "IPC timeout",
    "No IPC partner",
    "IPC canceled",
    "IPC message overflow",
    "Data transfer timed out (sender)",
    "Data transfer timed out (receiver)",
    "IPC aborted",
    "Invalid rights",
    "Invalid thread",
    "Invalid space",
    "Invalid arguments",
    "Out of memory",
    "Out of range",
    "Not found",
    "Exist",
    "Try again",
    "Timeout",
    "Unknown",
};

