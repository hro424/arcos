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
/// @file   Services/File/Ext2/Ext2FsServer.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: Ext2FsServer.h 422 2008-09-30 08:47:18Z hro $

#ifndef ARC_SERVICES_FILE_EXT2_SERVER_H_
#define ARC_SERVICES_FILE_EXT2_SERVER_H_

#include <SelfHealingServer.h>
#include <Types.h>
#include <l4/types.h>

class Disk;
class Ext2Fs;
class Ext2Partition;
class Partition;

class Ext2FsServer : public SelfHealingSessionServer
{
private:
    Disk*           _disk;
    Partition*      _partition;
    Ext2Partition*  _e2p;
    Ext2Fs*         _e2fs;

protected:
    static const Int    DEFAULT_PORT = 0;
    static const Int    DEFAULT_DISK = 0;
    static const Int    DEFAULT_PARTITION = 0;
    static const char*  DEFAULT_DISK_SERVER;

    virtual stat_t HandleBegin(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleEnd(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandleGet(const L4_ThreadId_t& tid, L4_Msg_t& msg);
    virtual stat_t HandlePut(const L4_ThreadId_t& tid, L4_Msg_t& msg);

    Int AllocateFileContainer();
    void ReleaseFileContainer(Int i);
    stat_t Initialize0(Int argc, char* argv[]);

public:
    virtual const char* const Name() { return "ext2"; }
    virtual stat_t Initialize(Int argc, char* argv[]);
    virtual stat_t Recover(Int argc, char* argv[]);
    virtual stat_t Exit();
};

#endif // ARC_SERVICES_FILE_EXT2_SERVER_H

