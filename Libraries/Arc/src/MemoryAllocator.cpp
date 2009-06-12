/*
 *
 *  Copyright (C) 2007, Waseda University.
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
/// @file   Libraries/Arc/memoryAllocator.cc
/// @brief  A memory block allocator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @date   2007
///

//$Id: MemoryAllocator.cpp 349 2008-05-29 01:54:02Z hro $

#include <MemoryAllocator.h>
#include <Types.h>
#include <sys/Config.h>

extern void _malloc_init();
extern void *_malloc_align(size_t count, size_t align);
extern void _mfree(void *ptr);

void*
malloc_align(size_t count, size_t align)
{
    return _malloc_align(count, align);
}

void*
malloc(size_t count)
{
    if (count < PAGE_SIZE) {
        return malloc_align(count, 1);
    }
    else {
        return malloc_align(count, PAGE_SIZE);
    }
}

void
mfree(void *ptr)
{
    _mfree(ptr);
}

void*
operator new(size_t s)
{
    void* ptr = malloc(s);
    if (ptr == 0) {
        FATAL("operator new: out of memory");
    }
    return ptr;
}

void*
operator new[](size_t s)
{
    void* ptr = malloc(s);
    if (ptr == 0) {
        FATAL("operator new[]: out of memory");
    }
    return ptr;
}

void
operator delete(void* ptr)
{
    mfree(ptr);
}

void
operator delete[](void* ptr)
{
    mfree(ptr);
}

