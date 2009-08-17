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

#include <Debug.h>
#include <Session.h>
#include <Stream.h>
#include <String.h>
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

    FileStream() {}

    virtual ~FileStream() { Disconnect(); }

    virtual stat_t Connect(L4_ThreadId_t tid)
    {
        ENTER;

        _ss = new Session();
        if (_ss == 0) {
            return ERR_OUT_OF_MEMORY;
        }

        _ss->Connect(tid);

        if (!_ss->IsConnected()) {
            delete _ss;
            _ss = 0;
            return ERR_UNKNOWN;
        }

        EXIT;
        return ERR_NONE;
    }

    virtual void Disconnect()
    {
        if (_ss != 0) {
            delete _ss;
            _ss = 0;
        }
    }

    virtual stat_t Open(const char *path, UInt mode);

    virtual void Close()
    {
        if (_ss != 0 && _ss->End(0, 0) != ERR_NONE) {
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

    virtual stat_t Read(void* buffer, size_t count, size_t* rsize)
    {
        addr_t      ptr;
        UInt        offset;
        Int         length;
        Int         len;
        stat_t      err;

        ENTER;

        if (buffer == 0) {
            return ERR_INVALID_ARGUMENTS;
        }

        if (count == 0) {
            if (rsize != 0) {
                *rsize = 0;
            }
            return ERR_NONE;
        }

        ptr = reinterpret_cast<addr_t>(buffer);
        offset = _offset;
        length = (Int)count;

        while (0 < length) {
            L4_Word_t   reg[2];
            Int         ssize = static_cast<Int>(_ss->Size());

            len = (length < ssize) ? length : ssize;

            reg[0] = len;
            reg[1] = offset;

            err = _ss->Get(reg, 2, reg, 1);
            if (err != ERR_NONE) {
                return err;
            }

            Int read = reg[0];

            //DOUT("copy 0x%lX -> %p\n", _ss->GetBaseAddress(), ptr);
            memcpy(reinterpret_cast<void*>(ptr),
                   reinterpret_cast<const void*>(_ss->GetBaseAddress()), read);

            /*
            char* dptr = (char*)(_ss->GetBaseAddress());
            for (int i = 0; i < 1024; i += 16) {
                for (int j = 0; j < 16; j++) {
                    Debug.Print("%2X ", dptr[i + j] & 0xFF);
                }
                Debug.Print("\n");
            }
            Debug.Print("\n");

            dptr = (char*)(ptr);
            DOUT("%p\n", dptr);
            for (int i = 0; i < read; i += 16) {
                for (int j = 0; j < 16; j++) {
                    Debug.Print("%2X ", dptr[i + j] & 0xFF);
                }
                Debug.Print("\n");
            }
            Debug.Print("\n");
            */

            ptr += read;
            offset += read;
            length -= read;

            if (read < len) {
                break;
            }
        }

        //SessionSetExtra(_ss, (void *)offset);
        _offset = offset;
        if (rsize != 0) {
            *rsize = static_cast<size_t>(ptr - (addr_t)buffer);
        }

        EXIT;
        return ERR_NONE;
    }

    virtual void Write(Int c) { /* not implemented */ }

    virtual stat_t Write(const void* buf, size_t count, size_t* wsize);

    virtual void Flush() { /* not implemented */ }

    virtual Int Seek(Int offset, UInt mode)
    {
        switch (mode) {
            case SEEK_SET:
                _offset = offset;
                break;
            case SEEK_CUR:
                _offset += offset;
                break;
            case SEEK_END:
                _offset = _size + offset;
                break;
            default:
                break;
        }

        return _offset;
    }

    virtual Int Size() { return _size; }
};

#endif // ARC_FILE_STREAM_H

