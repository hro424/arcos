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
##  @file   Services/Devices/Audio/AC97/CMakeLists.txt
##  @since  July 2009
##

include_directories(
    ${CMAKE_SOURCE_DIR}/Libraries/Audio/include
    ${CMAKE_SOURCE_DIR}/Libraries/SelfHealing/include)

set(MODULE_NAME ac97)

add_definitions(-DSYS_DEBUG)
add_definitions(-DSYS_DEBUG_CALL)

include(${CMAKE_SOURCE_DIR}/Tools/CMake/Lv1Service.cmake)

add_custom_target(${MODULE_NAME}_rd
    COMMAND ${MKRD} -a -o ${CMAKE_BINARY_DIR}/${RAMDISK} ${MODULE_NAME})
add_dependencies(${MODULE_NAME}_rd ramdisk_pre)
add_dependencies(${MODULE_NAME}_rd ${MODULE_NAME})
add_dependencies(ramdisk ${MODULE_NAME}_rd)

