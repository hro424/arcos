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
/// @file   Applications/FsTest/App.cc
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  2007
///

// $Id: App.cc 369 2008-08-04 08:52:12Z hro $

//#define SYS_DEBUG_CALL

#include <Debug.h>
#include <FileStream.h>
#include <Ipc.h>
#include <Session.h>
#include <String.h>
#include <Types.h>
#include <System.h>
#include <sys/Config.h>
#include <l4/types.h>

#define PRINT   System.Print

class FileSystemTest
{
protected:
    static const size_t BUFFER_SIZE = PAGE_SIZE;

    char                testbuf[BUFFER_SIZE];
    FileStream          _stream;

    L4_ThreadId_t GetFileSystem(const char* name);

    bool OpenClose(const char* path, int count);
    bool OpenReadClose(const char* path, int count);

public:
    FileSystemTest(const char* name);
    ~FileSystemTest();
    void Run();
};

L4_ThreadId_t
FileSystemTest::GetFileSystem(const char* name)
{
    L4_Msg_t        msg;
    L4_ThreadId_t   ns;
    L4_ThreadId_t   tid;
    L4_Word_t       len;
    stat_t          err;

    ENTER;

    L4_Put(&msg, MSG_ROOT_NS, 0, 0, 0, 0);
    err = Ipc::Call(L4_Pager(), &msg, &msg);
    if (err != ERR_NONE) {
        return L4_nilthread;
    }

    ns.raw = L4_Get(&msg, 0);

    len = (strlen(name) + 1 + sizeof(L4_Word_t) - 1) / sizeof(L4_Word_t);
    if (len >= __L4_NUM_MRS) {
        len = __L4_NUM_MRS - 1;
    }

    L4_Put(&msg, MSG_NS_SEARCH, len, (L4_Word_t *)name, 0, 0);

    err = Ipc::Call(ns, &msg, &msg);
    if (err != ERR_NONE) {
        DOUT("fail: %s\n", stat2msg[err]);
        return L4_nilthread;
    }

    tid.raw = L4_Get(&msg, 0);

    EXIT;
    return tid;
}

bool
FileSystemTest::OpenClose(const char* path, int count)
{
    stat_t      err;

    PRINT("Open exisiting files ... ");
    for (int i = 0; i < count; i++) {
        // Open an existing file
        err = _stream.Open(path, FileStream::READ);
        if (err != ERR_NONE) {
            PRINT("not found\n");
            return false;
        }

        // Close the file
        _stream.Close();
    }
    PRINT("OK\n");

    PRINT("Open non-existing files ... ");
    for (int i = 0; i < count; i++) {
        // Open a non-existing file -> File not found
        err = _stream.Open(path, FileStream::READ);
        if (err == ERR_NONE) {
            PRINT("found\n");
            return false;
        }

        // Close a non-existing file -> Warning
        _stream.Close();
    }
    PRINT("OK\n");

    /*
    File        file2;

    if (file.Open(path) != ERR_NONE) {
        return false;
    }

    // This fails because the same file is already opened.
    if (file2.Open(path) == ERR_NONE) {
        return false;
    }

    if (file.Close() != ERR_NONE) {
        return false;
    }
    */

    return true;
}


bool
FileSystemTest::OpenReadClose(const char* path, int count)
{
    size_t  rsize;

    PRINT("Open-read-close ... ");
    for (int i = 0; i < count; i++) {
        // Open an existing file
        if (_stream.Open(path, FileStream::READ) != ERR_NONE) {
            return false;
        }

        if (_stream.Read(testbuf, BUFFER_SIZE, &rsize) != ERR_NONE) {
            return false;
        }

        _stream.Close();
    }
    PRINT("OK\n");

    return true;
}

/*
void
Test_OpenWriteClose(int count)
{
    File    file;
    size_t  wsize;

    for (int i = 0; i < count; i++) {
        // Open an existing file
        if (file.Open(path) != ERR_NONE) {
            return false;
        }

        if (file.Write(buf, count, &wsize) != ERR_NONE) {
            return false;
        }

        if (file.Close() != ERR_NONE) {
            return false;
        }
    }

    return true;
}

void
Test_OpenAppendClose(int count)
{
    File    file;
    size_t  wsize;

    for (int i = 0; i < count; i++) {
        if (file.Open(path) != ERR_NONE) {
            return false;
        }

        if (file.Write(buf, count, &wsize) != ERR_NONE) {
            return false;
        }

        if (file.Close() != ERR_NONE) {
            return false;
        }
    }

    return true;
}

bool
Test_CreateDelete1(int count)
{
    File    file;

    for (int i = 0; i < count; i++) {
        // 1.   Create a complete new file
        if (file.Create(path) != ERR_NONE) {
            return false;
        }

        //      Delete the file
        if (file.Delete(path) != ERR_NONE) {
            return false;
        }
    }

    for (int i = 0; i < count; i++) {
        if (file.Create(path) != ERR_NONE) {
            return false;
        }
        
        if (file.Close() != ERR_NONE) {
            return false;
        }

        if (file.Delete(path) != ERR_NONE) {
            return false;
        }
    }

    for (int i = 0; i < count; i++) {
        if (file.Create(path) != ERR_NONE) {
            return false;
        }

        if (file.Delete() != ERR_NONE) {
            return false;
        }
    }

    return true;
}

void
Test_CreateDelete2(int count)
{
    // 2.   Try to create an existing file -> Error
    //      Delete a non existing file -> Error

    for (int i = 0; i < count; i++) {
        if (file.Create(path) == ERR_NONE) {
            return false;
        }
    }

    for (int i = 0; i < count; i++) {
        if (file.Delete() == ERR_NONE) {
            return false;
        }
    }

    for (int i = 0; i < count; i++) {
        if (file.Delete(path) == ERR_NONE) {
            return false;
        }
    }
}

void
Test_Copy()
{
    File    src;
    File    dest;

    for (int i = 0; i < count; i++) {
        if (src.Open(path) != ERR_NONE) {
            return false;
        }

        if (dest.Open(dest_path) != ERR_NONE) {
            return false;
        }

        if (src.Read(buf, count, &rsize) != ERR_NONE) {
            return false;
        }

        if (dest.Write(buf, count, &wsize) != ERR_NONE) {
            return false;
        }

        if (src.Close() != ERR_NONE) {
            return false;
        }

        if (dest.Close() != ERR_NONE) {
            return false;
        }
    }

    return true;
}

void
Test_Move()
{
    File    src;
    File    dest;
    size_t  rsize;
    size_t  wsize;

    for (int i = 0; i < count; i++) {
        if (src.Open(src_path) != ERR_NONE) {
            return false;
        }

        if (dest.Open(dest_path) != ERR_NONE) {
            return false;
        }

        if (src.Read(buf, count, &rsize) != ERR_NONE) {
            return false;
        }

        if (dest.Write(buf, count, &wsize) != ERR_NONE) {
            return false;
        }

        if (src.Delete() != ERR_NONE) {
            return false;
        }

        if (dest.Close() != ERR_NONE) {
            return false;
        }
    }

    return true;
}
*/

FileSystemTest::FileSystemTest(const char* name)
{
    L4_ThreadId_t   tid = GetFileSystem(name);

    if (_stream.Connect(tid) != ERR_NONE) {
        FATAL("fs not found");
    }
}

FileSystemTest::~FileSystemTest()
{
    _stream.Disconnect();
}

void
FileSystemTest::Run()
{
    const char* path = "/test1/hello.txt";
    OpenClose(path, 10);
    OpenReadClose(path, 10);
}

int
main()
{
    FileSystemTest t("ext2");
    t.Run();
    PRINT("end of test\n");
    return 0;
}

