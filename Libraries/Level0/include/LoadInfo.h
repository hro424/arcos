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
/// @brief  The information of the user program
/// @file   Include/arc/lv0/LoadInfo.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: LoadInfo.h 387 2008-08-30 05:07:58Z hro $

#ifndef ARC_LEVEL0_LOADINFO_H
#define ARC_LEVEL0_LOADINFO_H

#include <Types.h>
#include <TaskMap.h>

struct TaskMap;

struct LoadInfo
{
    ///
    /// Indicates the program is already loaded in the main memory. 
    ///
    static const UInt   LOAD_INFO_MEM = 0x01;

    ///
    /// Indicates that the program must be loaded from a disk.
    ///
    static const UInt   LOAD_INFO_FILE = 0x02;

    ///
    /// Indicates the module in the main memory is an AR archive.
    ///
    static const UInt   LOAD_INFO_MODULE = 0x03;

    ///
    /// Indicates fault injection
    ///
    static const UInt   LOAD_INFO_FI = 0x10;

    ///
    /// The type of LoadInfo
    ///
    UInt      type;

    ///
    /// The object that holds the memory layout of the program code
    ///
    TaskMap   task_map;
};

#endif // ARC_LEVEL0_LOADINFO_H

