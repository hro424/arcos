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
/// @brief  The information of the user program on memory
/// @file   Include/arc/lv0/TaskMap.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  December 2007
///

//$Id: TaskMap.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_LEVEL0_TASKMAP_H
#define ARC_LEVEL0_TASKMAP_H

#include <Types.h>

struct TaskMap
{
    /// 
    /// The beginning of the text section
    ///
    addr_t  text_start;

    ///
    /// The size of the text section in bytes
    ///
    size_t  text_size;

    ///
    /// The beginning of the data section
    ///
    addr_t  data_start;

    ///
    /// The size of the data section
    ///
    size_t  data_size;

    ///
    /// The beginning of the persistent section
    ///
    addr_t  pm_start;

    ///
    /// The size of the persistent section
    ///
    size_t  pm_size;

    ///
    /// The entry point of the task
    ///
    addr_t  entry;

    TaskMap() : text_start(0), text_size(0), data_start(0), data_size(0),
                pm_start(0), pm_size(0) {}
};

#endif // ARC_LEVEL0_TASKMAP_H

