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
/// @brief  A media streaming simulator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  August 2008
///

//$Id: App.cpp 394 2008-09-02 11:49:53Z hro $

#include <Debug.h>
#include <Types.h>
#include <FileStream.h>
#include <MemoryAllocator.h>
#include <NameService.h>
#include <System.h>
#include <l4/ipc.h>

///
/// Time for decoding the data
///
#define DECODING_TIME       5000

#define KB(v)               ((v) * 1024)
#define MB(v)               (KB(v) * 1024)

///
/// The size of buffer
///
#define BUFFER_SIZE         KB(20)

///
/// The size of a frame
/// X * Y * Depth
///
#define FRAME_SIZE          (640 * 480 * 16)

///
/// Compression rate
///
#define COMP_RATE           10

///
/// Frame per second
///
#define FPS                 30

///
/// Decoding time period in micro-second
///
#define DECODING_PERIOD     33000UL

static long counter;



static inline unsigned long long
ReadTSC()
{
    unsigned long long c;
    // edx:eax
    asm volatile ("rdtsc" : "=A" (c));
    return c;
}

static inline void
ReadTSC(unsigned long* hi, unsigned long* lo)
{
    asm volatile ("rdtsc" : "=a" (*lo), "=d" (*hi));
}

static inline unsigned long
ReadTSCHi()
{
    unsigned long c;
    asm volatile ("rdtsc" : "=a" (c));
    return c;
}

static inline unsigned long
ReadTSCLo()
{
    unsigned long c;
    asm volatile ("rdtsc" : "=d" (c));
    return c;
}

static unsigned long time0_hi;
static unsigned long time0_lo;
static unsigned long time1_hi;
static unsigned long time1_lo;

// Read 33ms of data
// Decode
// Write the data to a device (the device processes 10ms of data)
static int
ProcessFrame(FileStream& fstream, char* buffer, size_t size)
{
    stat_t  err;
    size_t  frame_size = FRAME_SIZE / COMP_RATE;
    size_t  len = size;
    size_t  rsize;
    size_t  total = 0;

    System.Print("Process frame %d\n", counter);
    ReadTSC(&time0_hi, &time0_lo);
    for (size_t i = 0; i < (FRAME_SIZE + size - 1) / size; i++) {
        err = fstream.Read(buffer, len, &rsize);
        if (rsize == 0) {
            return 0;
        }
        total += rsize;

        if (frame_size > BUFFER_SIZE) {
            frame_size -= BUFFER_SIZE;
            if (frame_size < BUFFER_SIZE) {
                len = frame_size;
            }
        }
        else {
            break;
        }

        // Do some work
        //L4_Sleep(L4_TimePeriod(DECODING_TIME));
    }
    ReadTSC(&time1_hi, &time1_lo);
    counter++;

    DOUT("%lu:%lu::%lu:%lu\n", time0_hi, time0_lo, time1_hi, time1_lo);

    return total;
}

static void
SkipNextFrame(FileStream& fstream)
{
    System.Print("Skip frame %d\n", counter);
    fstream.Seek(FRAME_SIZE, FileStream::SEEK_CUR);
    counter++;
}

static unsigned long
GetCurrentTime()
{
    return 0;
}

static void
Play(FileStream& fstream)
{
    unsigned long   time0;
    unsigned long   time1;
    unsigned long   diff;
    char*           buffer;

    buffer = reinterpret_cast<char*>(malloc(BUFFER_SIZE));
    counter = 0;

    for (;;) {
        time0 = GetCurrentTime();
        if (ProcessFrame(fstream, buffer, BUFFER_SIZE) == 0) {
            break;
        }
        time1 = GetCurrentTime();
        //diff = time1 - time0;
        diff = DECODING_TIME;
        if (diff <= DECODING_PERIOD) {
            //L4_Sleep(L4_TimePeriod(DECODING_PERIOD - diff));
        }
        else {
            for (unsigned long i = 0; i < diff / DECODING_PERIOD; i++) {
                SkipNextFrame(fstream);
            }
        }
    }

    mfree(buffer);
}

int
main(int argc, char* argv[])
{
    stat_t          err;
    L4_ThreadId_t   fs_id;
    FileStream      fstream;

    if (argc < 3) {
        System.Print("usage: player <server> <file>\n");
        return 0;
    }

    System.Print("look up '%s'\n", argv[1]);

    err = NameService::Get(argv[1], &fs_id);
    if (err != ERR_NONE) {
        return err;
    }

    System.Print("fs id %.8lX\n", fs_id.raw);

    err = fstream.Connect(fs_id);
    if (err != ERR_NONE) {
        return err;
    }

    System.Print("open %s\n", argv[2]);

    err = fstream.Open(argv[2], FileStream::READ);
    if (err != ERR_NONE) {
        return err;
    }

    Play(fstream);

    fstream.Close();

    fstream.Disconnect();
    System.Print("fs disconnected\n");

    return 0;
}

