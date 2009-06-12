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
/// @file   Services/Root/Pager.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2006
///

// $Id: Pager.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_ROOT_PAGER_H
#define ARC_ROOT_PAGER_H

#include <Types.h>
#include <l4/types.h>
#include <l4/message.h>

class PageFrame;
class PageFrameTable;

class Pager {
private:
    PageFrameTable          *_pft;

    L4_Word_t ProbeKernelArea();

    stat_t MapSigma0(L4_Fpage_t fpage, L4_Fpage_t backing);

    stat_t CreateMap(addr_t         address,
                     L4_Word_t      rwx,
                     addr_t         sndbase,
                     L4_MapItem_t*  map);

public:
    Pager() { _pft = 0; }

    void SetPft(PageFrameTable *pft) { _pft = pft; }

    ///
    /// Fills out the page with zero.
    ///
    /// @param dest     the page frame object that corresponds to the page
    ///
    void ZeroPage(PageFrame *dest);

    ///
    /// Copies the page frame and the page content.
    ///
    void CopyPage(PageFrame *dest, PageFrame *src);

    ///
    /// Using Sigma0 protocol, asks Sigma0 or the parent pager to map the
    /// specified region in Sigma0 onto the region in the pager's address space.
    ///
    /// @param fpage    the fpage to be backed
    /// @param backing  the parent's fpage
    ///
    Bool IsValidAddress(addr_t address);

    ///
    /// Creates a mapping information object.  An actual page mapping is done
    /// by sending the mapping information object to an address space via IPC.
    ///
    /// @param dest     the base address of the destination
    /// @param frame    the page frame to be mapped
    /// @param rwx      the permission to be given to the destination
    /// @param count    the number of pages
    /// @param item     the mapping information
    ///
    stat_t CreateMapItem(addr_t             dest,
                         PageFrame*         frame,
                         L4_Word_t          rwx,
                         L4_Word_t          count,
                         L4_MapItem_t*      items);

    ///
    /// Creates a mapping information object.  An actual page mapping is done
    /// by sending the mapping information object to an address space via IPC.
    ///
    /// @param dest     the base address of the destination
    /// @param frame    the page frame to be mapped
    /// @param rwx      the permission to be given to the destination
    /// @param item     the mapping information
    ///
    stat_t CreateMapItem(addr_t             dest,
                         PageFrame*         frame,
                         L4_Word_t          rwx,
                         L4_MapItem_t*      item);

    ///
    /// Discards the mapping backed by the specified page.
    ///
    /// @param frame    the page frame of the backing page
    /// @param rwx      the access permission to be dropped
    ///
    stat_t Unmap(PageFrame*         frame,
                 L4_Word_t          rwx);

    ///
    /// Obtains the physical address that corresponds to the given page frame.
    ///
    addr_t PhysicalAddress(PageFrame* frame);
};

#endif // ARC_ROOT_PAGER_H

