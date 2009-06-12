/*
 *
 *  Copyright (C) 2008, Waseda University. All rights reserved.
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

//$Id: stdarg.h 325 2008-05-09 04:22:15Z hro $

#ifndef _STDARG_H_
#define _STDARG_H_

#define va_size(type)   \
    (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#if defined(__GNUC__) && __GNUC__ >= 3

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list   __gnuc_va_list;
#endif // __GNUC_VA_LIST

#define va_start(ap, last)  __builtin_stdarg_start((ap), last)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy((dest),(src))

typedef __gnuc_va_list      va_list;

#else // !__GNUC__

//XXX: arch dependent (IA32)
#define va_start(ap, last)  \
    ((ap) = (va_list) & (last) + va_size(last))
#define va_arg(ap, type)    \
    (*(type*)((ap) += va_size(type), (ap) - va_size(type)))
#define va_copy(dest, src)  \
    ((void)((dest) = (src)))
#define va_end(ap)

#endif // __GNUC__

#endif // _STDARG_H_

