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
/// @file   Include/arc/nameservice.cc
/// @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
/// @since  2008
/// 

//$Id: NameService.h 383 2008-08-28 06:49:02Z hro $

#ifndef ARC_NAMESERVICE_H_
#define ARC_NAMESERVICE_H_

#include <Ipc.h>
#include <Types.h>
#include <l4/thread.h>

/**
 * Defines helper functions to use the naming service.
 */
class NameService {
private:
    static L4_ThreadId_t root_ns;
    static inline L4_ThreadId_t rootNS()
    {
        if (!root_ns.raw) {
            L4_Msg_t msg;
            stat_t err;
            L4_Put(&msg, MSG_ROOT_NS, 0, 0, 0, 0);
            err = Ipc::Call(L4_Pager(), &msg, &msg);
            if (err == ERR_NONE)
                root_ns.raw = L4_MsgWord(&msg, 0);
        }
        
        return root_ns;
    }
public:
    /**
     * Associates a thread id with a litteral name.
     */
    static stat_t Insert(const char *const name, const L4_ThreadId_t tid);
    
    /**
     * Fetches the thread id associated with the given name.
     */
    static stat_t Get(const char *const name, L4_ThreadId_t *const tid);
    
    /**
     * Remove the NS entry associated with name.
     * @note Not Yet Implemented in root server.
     */
    static stat_t Remove(const char *const name);

    static stat_t List();
};

#endif // ARC_NAMESERVICE_H_
