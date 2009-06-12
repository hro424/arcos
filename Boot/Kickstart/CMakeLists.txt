##
##  Copyright (C) 2007, 2008, Waseda University.
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
##  @file   Boot/Kickstart/CMakeLists.txt
##  @date   2007
##

#$Id: CMakeLists.txt 406 2008-09-08 05:31:26Z hro $

include_directories(
    ${L4_SOURCEDIR}/user/include
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Include/arch/${ARCH}
    ${CMAKE_SOURCE_DIR}/Libraries/C/include)

set(PROG kickstart)
file(GLOB SRCS *.S *.c)
set(CMAKE_C_LINK_FLAGS "-Ttext=${KICKSTART_LINKBASE} --script ${CMAKE_CURRENT_SOURCE_DIR}/kickstart.ld")

add_executable(${PROG} ${SRCS})
target_link_libraries(${PROG} c gcc)

# Copy Kickstart to the disk image
add_custom_target(${PROG}_rd
    COMMAND ${E2CP} ${PROG} ${CMAKE_BINARY_DIR}/${ROOT_FS}:${BOOT_DIR})
add_dependencies(${PROG}_rd ${PROG})
add_dependencies(${PROG}_rd rootdisk_pre)
add_dependencies(rootdisk_post ${PROG}_rd)

install(TARGETS ${PROG} DESTINATION .)

