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
/// @file   Include/arc/FileStream.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: FileStream.h 385 2008-08-28 18:38:11Z hro $

#ifndef ARC_FILE_STREAM_H
#define ARC_FILE_STREAM_H

#include <Session.h>
#include <Stream.h>
#include <Types.h>
#include <l4/types.h>

class FileStream : public Stream
{
protected:
    Session*    _ss;
    Int         _offset;
    Int         _size;

public:
    enum Mode {
        // Creates a file
        CREATE =        0x001,
        // Write to a file
        WRITE =         0x002,
        // Read a file
        READ =          0x004,
        // Trancate the file length to 0
        TRUNCATE =      0x010,
        // Write to a file.  Expand it if data exceeds the file.
        APPEND =        0x020,
        //
        EXISTENCE =     0x040,
    };

    enum SeekMode {
        SEEK_SET =      0,
        SEEK_CUR =      1,
        SEEK_END =      2,
    };

    virtual stat_t Connect(L4_ThreadId_t tid);

    virtual void Disconnect() { delete _ss; }

    virtual stat_t Open(const char *path, UInt mode);

    virtual void Close()
    {
        if (_ss->End(0, 0) != ERR_NONE) {
            //XXX: Recovery
        }
    }

    virtual void Lock() {}

    virtual void Unlock() {}

    virtual Int Read()
    {
        Int     buf;
        size_t  rsize;

        if (Read(&buf, sizeof(Int), &rsize) == ERR_NONE) {
            return buf;
        }
        else {
            return 0;
        }
    }

    virtual stat_t Read(void* buf, size_t count, size_t* rsize);

    virtual void Write(Int c) { /* not implemented */ }

    virtual stat_t Write(const void* buf, size_t count, size_t* wsize);

    virtual void Flush() { /* not implemented */ }

    virtual Int Seek(Int offset, UInt mode);

    virtual Int Size() { return _size; }
};

#endif // ARC_FILE_STREAM_H

