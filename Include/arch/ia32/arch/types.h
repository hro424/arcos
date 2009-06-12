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
/// @file   Include/arc/types.h
/// @brief  Primitive types used in Arc services
/// @since  2007
///

//$Id: types.h 325 2008-05-09 04:22:15Z hro $

#ifndef ARC_IA32_TYPES_H
#define ARC_IA32_TYPES_H

typedef char                Byte;           // 1 byte
typedef unsigned char       UByte;
typedef short               Short;          // 2 bytes
typedef unsigned short      UShort;
typedef long                Int;            // 4 bytes
typedef unsigned long       UInt;
typedef long long           Long;           // 8 bytes
typedef unsigned long long  ULong;

typedef Byte                Bool;
const Bool                  FALSE = 0;
const Bool                  TRUE = !FALSE;

typedef unsigned long       word_t;
typedef word_t              addr_t;

typedef unsigned int        size_t;
typedef int                 ssize_t;

#endif // ARC_IA32_TYPES_H

