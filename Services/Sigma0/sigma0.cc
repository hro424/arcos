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
/// @file   Services/System/Sigma0/sigma0.cpp
/// @brief  Sigma0 implementation
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

//$Id: sigma0.cc 429 2008-11-01 02:24:02Z hro $

#include <DebugStream.h>
#include <Stream.h>
#include <System.h>
#include <Ipc.h>
#include <Types.h>

#include <l4/ipc.h>
#include <l4/kip.h>
#include <l4/types.h>
#include <l4/kdebug.h>

#define L4_REQUEST_MASK     (~((~0UL) >> (L4_BITS_PER_WORD - 20)))
#define L4_PROTO_PF         (-2U << 20)
#define L4_PROTO_SIGMA0     (-6U << 20)

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL 0
#endif // VERBOSE_LEVEL

SystemHelper System(VERBOSE_LEVEL, &__dstream);

// Dummy
void operator delete(void* ptr) { L4_KDB_Enter("sigma0: operater delete"); }

void
__exit(int stat)
{
}

static inline void
PrintMemoryInfo()
{
    System.Print("     Begin      End     Type  Usage\n");
    System.Print("--- --------   -------- ---- ------------------\n");

    L4_KernelInterfacePage_t* kip =
            static_cast<L4_KernelInterfacePage_t *>(L4_GetKernelInterface());

    for (UInt i = 0; i < L4_NumMemoryDescriptors(kip); i++) {
        L4_MemoryDesc_t *mem_desc;
        L4_Word_t	low, high;

        mem_desc = L4_MemoryDesc(kip, i);

        low = L4_MemoryDescLow(mem_desc);
        high = L4_MemoryDescHigh(mem_desc);

        System.Print("%2lu: %.8lX - %.8lX ", i, low, high);
        if (L4_IsMemoryDescVirtual(mem_desc)) {
            System.Print("virt ");
        }
        else {
            System.Print("phys ");
        }

        System.Print("%.8lX ", L4_MemoryDescType(mem_desc));

        switch (L4_MemoryDescType(mem_desc) & 0xF) {
        case L4_UndefinedMemoryType:
            System.Print("undefined\n");
            break;
        case L4_ConventionalMemoryType:
            System.Print("conventional\n");
            break;
        case L4_ReservedMemoryType:
            System.Print("reserved\n");
            break;
        case L4_DedicatedMemoryType:
            System.Print("dedicated\n");
            break;
        case L4_SharedMemoryType:
            System.Print("shared\n");
            break;
        case L4_BootLoaderSpecificMemoryType:
            System.Print("bootloader\n");
            break;
        case L4_ArchitectureSpecificMemoryType:
            System.Print("architecture\n");
            break;
        default:
            System.Print("Unknown memory type %lX\n",
                           L4_MemoryDescType(mem_desc));
            continue;
        }
    }
}

///
/// Handles page fault protocol. Perform direct mapping of the requested page
/// to the same address.
/// 
static inline stat_t
HandlePageFault(L4_ThreadId_t from, L4_Msg_t *msg)
{
    L4_Fpage_t      fpage;
    L4_MapItem_t    map;
    L4_Word_t       faddr, fip;

    if (Ipc::CheckPayload(msg, 0, 2)) {
        return ERR_INVALID_ARGUMENTS;
    }

    // Get the fault address
    faddr = L4_Get(msg, 0);

    // Get the fault instruction pointer
    fip = L4_Get(msg, 1);

    // Align the fault address to page size
    faddr &= PAGE_MASK;

    // Assign the corresponding page
    fpage = L4_FpageLog2(faddr, PAGE_BITS);

    // Set up the mapping. Give full access permission.
    map = L4_MapItem(L4_FpageAddRights(fpage, L4_FullyAccessible), faddr);

    // Build the message to be sent the page to the client
    L4_Put(msg, 0, 0, 0, 2, &map);

    return ERR_NONE;
}

///
/// Handles S0 protocol. Currently only supports direct mapping of requested
/// pages.
/// 
static inline stat_t
HandleS0Protocol(L4_ThreadId_t from, L4_Msg_t *msg)
{
    L4_Fpage_t      req;
    L4_MapItem_t    map;
    L4_Word_t       tmp;
    L4_Word_t       reg[2];
    L4_Word_t       base;
    L4_KernelInterfacePage_t    *kip;

    kip = (L4_KernelInterfacePage_t*)L4_GetKernelInterface();
    base = L4_ThreadIdUserBase(kip);

    // Get the requested page
    req.raw = L4_Get(msg, 0);

    // S0 Kernel Protocol
    tmp = req.X.b;
    if (tmp == -1UL) {
        //TODO: S0 decides which page is offered to kernel.
        if (0 == req.X.s) {
            reg[0] = PAGE_SIZE;
            reg[1] = 0;
            L4_Put(msg, 0, 2, reg, 0, 0);
        }
        // S0 grants the specified memory region to kernel.
        else {
            /*
            L4_Fpage_t	fpage;

            if (0 != MM_find(req, L4_nilthread, &fpage)) {
            	L4_GrantItem_t	item;
            	item = L4_GrantItem((fpage), 0);
            	L4_MsgPut(&msg, 0, 0, (L4_Word_t *)0, 2, &item);
            }
            // Rejects grant
            else {
            	L4_Word_t reg[2] = {10UL, 0UL};
            	L4_MsgPut(&msg, 0, 0, (L4_Word_t *)0, 2, reg);
            }
            */
            System.Print(System.ERROR, "HandleS0Protocol_NotImplemented");
        }
    }
    // S0 User Protocol
    else {
        // Only ART threads are allowed
        if (L4_IsThreadNotEqual(from, L4_GlobalId(base + 0x02, 1)) && 
            L4_IsThreadNotEqual(from, L4_GlobalId(base + 0x03, 1)) && 
            L4_IsThreadNotEqual(from, L4_GlobalId(base + 0x04, 1))) {
            return ERR_INVALID_SPACE;
        }

        // Give full access permission
        map = L4_MapItem(L4_FpageAddRights(req, L4_FullyAccessible), 0);
        L4_Put(msg, 0, 0, 0, 2, &map);
    }

    return ERR_NONE;
}

///
/// Handles IPC, discriminating between the different protocols understood by
/// sigma0.
///
static void
HandleIpc()
{
    L4_Msg_t        msg;
    L4_MsgTag_t     tag;
    L4_ThreadId_t   tid;

    L4_Accept(L4_UntypedWordsAcceptor);

begin:
    tag = L4_Wait(&tid);

    for (;;) {
        if (L4_IpcFailed(tag)) {
            goto begin;
        }

        L4_Store(tag, &msg);
        switch (tag.raw & L4_REQUEST_MASK) {
            case L4_PROTO_PF:   // Handle page fault
                if (HandlePageFault(tid, &msg) != ERR_NONE) {
                    L4_Clear(&msg);
                }
                break;
            case L4_PROTO_SIGMA0:   // Handle the sigma0 protocol
                if (HandleS0Protocol(tid, &msg) != ERR_NONE) {
                    L4_Clear(&msg);
                    return;
                }
                break;
            default:    // Do nothing, wait for the next request
                System.Print(System.WARN,
                             "Sigma0: Unknown message: %.8lX from %.8lX\n",
                             L4_Label(&msg), tid.raw);
                break;
        }

        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
    }
}

int
main()
{
    PrintMemoryInfo();
    HandleIpc();
    return 0;
}

