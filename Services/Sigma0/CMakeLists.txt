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
##  @file   Services/Level0/Sigma0/CMakeLists.txt
##  @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
##  @since  2007
##

#$Id: CMakeLists.txt 409 2008-09-08 05:31:28Z hro $

include_directories(
    ${L4_SOURCEDIR}/user/include
    ${CMAKE_SOURCE_DIR}/Libraries/System/include)
set(MODULE_NAME sigma0)
set(MODULE_LINKBASE ${SIGMA0_LINKBASE})
set(VERBOSE_LEVEL 0)
set(CRT_SRC ${CMAKE_SOURCE_DIR}/Libraries/arch/${ARCH}/crt0-s0.S)
set(LDSCRIPT ${CMAKE_SOURCE_DIR}/Libraries/arch/${ARCH}/lv0.lds)
list(APPEND LIBS lv0 sys c++ l4 gcc)

include(${CMAKE_SOURCE_DIR}/Tools/CMake/Build.cmake)

# Add the module to the disk image
add_custom_target(${MODULE_NAME}_hd
    COMMAND ${E2CP} ${MODULE_NAME} ${CMAKE_BINARY_DIR}/${ROOT_FS}:${BOOT_DIR})
add_dependencies(${MODULE_NAME}_hd ${MODULE_NAME})
add_dependencies(${MODULE_NAME}_hd rootdisk_pre)
add_dependencies(rootdisk_post ${MODULE_NAME}_hd)

