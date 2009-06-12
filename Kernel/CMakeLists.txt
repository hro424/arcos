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
##  3. Neither the name of Waseda University nor the names of its contributors
##     may be used to endorse or promote products derived from this software
##     without specific prior written permission.
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
##  @file   Kernel/CMakeLists.txt
##  @since  May 2008
##

#$Id: CMakeLists.txt 406 2008-09-08 05:31:26Z hro $

set(MODULE_NAME l4mk)

find_program(HG hg)
find_program(MAKE make)
find_program(PATCH patch)
find_program(CAT cat)
find_program(CP cp)

# Download L4::Pistachio
if(NOT EXISTS ${L4_SOURCEDIR})
    message("Downloading L4::Pistachio ...")
    execute_process(COMMAND ${HG} clone ${L4_REPOSITORY} ${L4_SOURCEDIR})
    message("done.")

endif(NOT EXISTS ${L4_SOURCEDIR})

if(NOT EXISTS ${L4_BUILDDIR})
    # Prepare to build the kernel
    execute_process(
        COMMAND ${MAKE} BUILDDIR=${L4_BUILDDIR} 
        WORKING_DIRECTORY ${L4_SOURCEDIR}/kernel)
    execute_process(
        COMMAND ${PATCH} -p1
        INPUT_FILE ${PATCH_FILE}
        WORKING_DIRECTORY ${L4_BUILDDIR})
endif(NOT EXISTS ${L4_BUILDDIR})

# Build the kernel
add_custom_target(${MODULE_NAME} ALL
    COMMAND ${MAKE}
    WORKING_DIRECTORY ${L4_BUILDDIR})

# Copy the kernel to the disk image
add_custom_target(${MODULE_NAME}_rd
    COMMAND ${E2CP} ${L4_BUILDDIR}/x86-kernel ${CMAKE_BINARY_DIR}/${ROOT_FS}:${BOOT_DIR})
add_dependencies(${MODULE_NAME}_rd ${MODULE_NAME})
add_dependencies(${MODULE_NAME}_rd rootdisk_pre)
add_dependencies(rootdisk_post ${MODULE_NAME}_rd)

install(PROGRAMS ${L4_BUILDDIR}/x86-kernel DESTINATION .)

# Build L4 library
include_directories(${L4_SOURCEDIR}/user/include)
if(${ARCH} STREQUAL ia32)
    list(APPEND SRCS ${L4_SOURCEDIR}/user/lib/l4/${ARCH}-syscall-init.cc ${L4_SOURCEDIR}/user/lib/l4/${ARCH}-syscall-stubs.S)
endif(${ARCH} STREQUAL ia32)

add_library(l4 STATIC ${SRCS})

set_target_properties(l4 PROPERTIES LINKER_LANGUAGE CXX)

