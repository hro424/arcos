/*
 *
 *  Copyright (C) 2006, 2007, 2008, Waseda University.
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
/// @file   Libraries/System/include/Types.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: Types.h 394 2008-09-02 11:49:53Z hro $

#ifndef ARC_TYPES_H
#define ARC_TYPES_H

#include <arch/types.h>

#define INLINE          inline static

enum stat_t {
    ERR_NONE = 0,                   // No error
    ERR_SYS_INVALID_RIGHTS = 1,     // No privilege
    ERR_SYS_INVALID_THREAD = 2,     // Invalid thread ID
    ERR_SYS_INVALID_SPACE = 3,      // Invalid space ID
    ERR_SYS_INVALID_SCHED = 4,      // Invalid scheduler ID
    ERR_SYS_INVALID_ARGUMENTS = 5,  // Invalid parameters
    ERR_SYS_INVALID_UTCB = 6,       // Invalid UTCB address
    ERR_SYS_INVALID_KIP = 7,        // Invalid KIP address
    ERR_SYS_OUT_OF_MEMORY = 8,      // Kernel failed to allocate memory.
    ERR_IPC_TIMEOUT = 9,            // IPC operation timed out.
    ERR_IPC_NO_PARTNER = 10,        // No IPC partner found.
    ERR_IPC_CANCELED = 11,          // IPC was canceled.
    ERR_IPC_OVERFLOW = 12,          // Message overflow
    ERR_IPC_TIMEOUT_SEND = 13,      // Data transfer timed out.
    ERR_IPC_TIMEOUT_RECV = 14,      // Data transfer timed out.
    ERR_IPC_ABORTED = 15,           // IPC was aborted.
    ERR_INVALID_RIGHTS = 16,        // Invalid privilege
    ERR_INVALID_THREAD = 17,        // Invalid thread ID
    ERR_INVALID_SPACE = 18,         // Invalid space ID
    ERR_INVALID_ARGUMENTS = 19,     // Invalid parameters
    ERR_OUT_OF_MEMORY = 20,         // Failed to allocate memory
    ERR_OUT_OF_RANGE = 21,          // Parameter out of range
    ERR_NOT_FOUND = 22,             // Resource not found
    ERR_EXIST = 23,                 // Resource is already allocated
    ERR_BUSY = 24,                  // Resrouce is occupied
    ERR_TIMEOUT = 25,               // Opration timed out
    ERR_FATAL = 26,
    ERR_UNKNOWN,                    // Unknown error
};

static const Int ERR_OFFSET_IPC = 8;    // Offset to IPC error space
static const Int NSTATUS = 27;
static const UInt STATUS_MASK = 0xFF;

extern const char* const stat2msg[];   // Error code-message converter

INLINE Bool
IsSystemError(stat_t err)
{
    return (Bool)(ERR_NONE < err && err < ERR_IPC_TIMEOUT);
}

INLINE Bool
IsIpcError(stat_t err)
{
    return (Bool)(ERR_SYS_OUT_OF_MEMORY < err && err < ERR_INVALID_RIGHTS);
}

INLINE const char* const
ErrToString(stat_t err)
{
    return stat2msg[err];
}

#endif // ARC_TYPES_H

