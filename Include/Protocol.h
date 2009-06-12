/*
 *
 *  Copyright (C) 2006, 2007, Waseda University.
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
/// @file   Include/arc/protocol.h
/// @brief  IPC protocol definitions
/// @since  2006
///

//$Id: Protocol.h 426 2008-10-01 09:38:07Z hro $

#ifndef ARC_PROTOCOL_H
#define ARC_PROTOCOL_H

#define MSG_MASK                        0xFFF0

//
//  Sigma0
//
#define MSG_SIGMA0                      ((-6U << 4) & 0xFFFF)

//
//  Root Pager
//
#define MSG_PAGER_PROTO(n)              (((n) << 4) & 0xFFFF)
#define MSG_PAGER_MASK                  MSG_PAGER_PROTO(~0UL)
// start thread
#define MSG_PAGER_TH                    MSG_PAGER_PROTO(0UL)
// interrupt
#define MSG_PAGER_INT                   MSG_PAGER_PROTO(-1UL)
// page fault
#define MSG_PAGER_PF                    MSG_PAGER_PROTO(-2UL)
// preemption
#define MSG_PAGER_PREEMPT               MSG_PAGER_PROTO(-3UL)
// exception
#define MSG_PAGER_EXCEPTION             MSG_PAGER_PROTO(-4UL)
// architecture specific
#define MSG_PAGER_ARCH                  MSG_PAGER_PROTO(-5UL)
// IO page fault
#define MSG_PAGER_IOPF	                MSG_PAGER_PROTO(-8UL)
// page grant
#define MSG_PAGER_GRANT                 MSG_PAGER_PROTO(-9UL)

// page allocation request
#define MSG_PAGER_ALLOCATE              MSG_PAGER_PROTO(-10UL)
// page release request
#define MSG_PAGER_RELEASE               MSG_PAGER_PROTO(-11UL)
// page mapping request
#define MSG_PAGER_MAP                   MSG_PAGER_PROTO(-12UL)
// page unmapping request
#define MSG_PAGER_UNMAP                 MSG_PAGER_PROTO(-13UL)
// page mapping request
#define MSG_PAGER_IOMAP                 MSG_PAGER_PROTO(-14UL)
// page unmapping request
#define MSG_PAGER_IOUNMAP               MSG_PAGER_PROTO(-15UL)
// Get physical address
#define MSG_PAGER_PHYS                  MSG_PAGER_PROTO(-16UL)
//
#define MSG_PAGER_EXIT                  MSG_PAGER_PROTO(-17UL)
// Expand the heap area
#define MSG_PAGER_EXPAND                MSG_PAGER_PROTO(-18UL)
// Map a page from the first megabyte of physical memory
// (containing BIOS area)
#define MSG_PAGER_MAPBIOSPAGE           MSG_PAGER_PROTO(-19UL)

//
//  Root Task
//
#define MSG_ROOT_NEW_INIT           0x0010
#define MSG_ROOT_NEW_TH             0x0020
#define MSG_ROOT_NEW_TSK            0x0030
#define MSG_ROOT_PRIO               0x0040
#define MSG_ROOT_STOP_TH            0x0050
#define MSG_ROOT_DEL_TH             0x0060
#define MSG_ROOT_DEL_TSK            0x0070
#define MSG_ROOT_SET_INT            0x0080
#define MSG_ROOT_UNSET_INT          0x0090
#define MSG_ROOT_NS                 0x00A0
#define MSG_ROOT_EXEC               0x00B0
#define MSG_ROOT_EXIT               0x00C0
#define MSG_ROOT_RESTART            0x00D0
#define MSG_ROOT_SNAPSHOT           0x00E0
#define MSG_ROOT_RESTORE            0x00F0

//
//  Shadow Task
//
#define MSG_PEL_SYSCALL             0x2010
#define MSG_PEL_RESTART             0x2020
#define MSG_PEL_TERMINATE           0x2030
#define MSG_PEL_PM_ALLOCATE         0x2040
#define MSG_PEL_PM_RELEASE          0x2050
#define MSG_PEL_ALLOCATE            MSG_PAGER_ALLOCATE
#define MSG_PEL_RELEASE             MSG_PAGER_RELEASE
#define MSG_PEL_MAP                 MSG_PAGER_MAP
#define MSG_PEL_UNMAP               MSG_PAGER_UNMAP
#define MSG_PEL_IOMAP               MSG_PAGER_IOMAP
#define MSG_PEL_IOUNMAP             MSG_PAGER_IOUNMAP
#define MSG_PEL_PHYS                MSG_PAGER_PHYS
#define MSG_PEL_START_TH            0x2200
#define MSG_PEL_CANCEL              0x2210
#define MSG_PEL_READY               0x2220

//
//  Naming Service
//
#define MSG_NS_INSERT               0x1000
#define MSG_NS_SEARCH               0x1010
#define MSG_NS_REMOVE               0x1020
#define MSG_NS_LIST                 0x1030

#define MSG_FILE_SEEK               0x4040

//
//  Session
//
#define MSG_SESSION_CONNECT         0x5010
#define MSG_SESSION_DISCONNECT      0x5020
#define MSG_SESSION_BEGIN           0x5030
#define MSG_SESSION_END             0x5040
#define MSG_SESSION_PUT             0x5050
#define MSG_SESSION_GET             0x5060
#define MSG_SESSION_PUT_ASYNC       0x5070

//
//  Event notification
//
#define MSG_EVENT_CONNECT           0x6010
#define MSG_EVENT_DISCONNECT        0x6020
#define MSG_EVENT_NOTIFY            0x6030
#define MSG_EVENT_TERMINATE         0x6040

#endif // ARC_PROTOCOL_H

