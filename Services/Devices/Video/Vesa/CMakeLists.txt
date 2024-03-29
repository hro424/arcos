##
##  Copyright (C) 2008, Waseda University.
##  All rights reserved.
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions
##  are met:
##
##  1. Redistributions of source code must retain the above copyright notice,
##     this list of conditions and the following disclaimer.
##  2. Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
##  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
##  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
##  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
##  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
##  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
##  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
##  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
##  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
##  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
##  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

##
##  @file   Services/Devices/Vesa/CMakeLists.txt
##  @author Alexandre Courbot <alex@dcl.info.waseda.ac.jp>
##  @date   2008
##

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Include)

#FILE(GLOB SRCS *.cc)
#SET(TEXT_PHYS 0x600000)
#SET(PROG vesa)
#NOTE: arc and arc-user depend on each other.
#LIST(APPEND LIB_LIST arc arc-user arc posix c_arc l4 x86emu)
#ADD_DEFINITIONS(-DSYS_DEBUG)
#ADD_DEFINITIONS(-DSYS_DEBUG_CALL)
#ADD_DEFINITIONS(-DNO_SYS_HEADERS)

#INCLUDE(${CMAKE_SOURCE_DIR}/Tools/CMake/ArcServer.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/Libraries/Vesa/include
    ${CMAKE_SOURCE_DIR}/Libraries/SelfHealing/include
    ${CMAKE_SOURCE_DIR}/Libraries/L4/include
    ${CMAKE_SOURCE_DIR}/Libraries/Arc/include
    ${CMAKE_SOURCE_DIR}/Libraries/X86Emu/include
    ${CMAKE_SOURCE_DIR}/Libraries/C/include)

set(MODULE_NAME vesa)
list(APPEND LIBS x86emu psession video)

add_definitions(-DNO_SYS_HEADERS)
add_definitions(-DSYS_DEBUG)
add_definitions(-DSYS_DEBUG_CALL)

include(${CMAKE_SOURCE_DIR}/Tools/CMake/Lv1Service.cmake)

