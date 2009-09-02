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
##  @file   CMakeLists.txt
##  @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
##  @since  November 2007
##


cmake_minimum_required(VERSION 2.6)

project(Arc2)

set(ARCH ia32)
set(KERNEL pistachio)

find_program(CHMOD chmod)
find_program(CP             cp)
find_program(DD             dd)
find_program(ECHO           echo)
find_program(E2MKDIR        e2mkdir)
find_program(E2CP           e2cp)
find_program(GRUB_INSTALL   install_grub
    PATHS ${CMAKE_SOURCE_DIR}/Tools/Scripts)
find_program(MKE2FS         mke2fs)
find_program(MKEMPTY        mkempty
    PATHS ${CMAKE_SOURCE_DIR}/Tools/Scripts)
find_program(MKRD           mkrd
    PATHS ${CMAKE_SOURCE_DIR}/Tools/Scripts)
find_program(MV             mv)
find_program(QEMU           qemu)
find_program(QEMU_IMAGE     qemu-img)
find_program(TOUCH          touch)
find_program(VMWARE         vmware)
find_program(WHOAMI         whoami)

##
##  Include config file
##
include(${CMAKE_SOURCE_DIR}/Tools/CMake/Config.cmake)

##
##  Include homemade CMake files
##
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Tools/CMake)


##
##  Enable assembly language
##
enable_language(ASM-GCC)
set(CMAKE_ASM-GCC_COMPILER_ARG1 -c) # invoke gcc with "-c" option


##
##  Find libgcc
##
exec_program(${CMAKE_CXX_COMPILER}
    ARGS -print-libgcc-file-name OUTPUT_VARIABLE LIBGCC)
string(REGEX REPLACE "/[^/]*$" "" LIBGCCDIR ${LIBGCC})


##
##  Set up the compiler option
##
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CXX_COMPILER_VERSION_MESSAGE)
string(REGEX MATCH "[0-9]\\.[0-9]\\.[0-9]"
    CXX_COMPILER_VERSION ${CXX_COMPILER_VERSION_MESSAGE})

if(${CXX_COMPILER_VERSION} STRGREATER 4.2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow  -Winline -g -O2 -nostdlib -nostdinc -fno-exceptions -fno-rtti -fno-stack-protector -fno-threadsafe-statics")
else(${CXX_COMPILER_VERSION} STRGREATER 4.2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow  -Winline -g -O2 -nostdlib -nostdinc -fno-exceptions -fno-rtti")
endif(${CXX_COMPILER_VERSION} STRGREATER 4.2)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wshadow -Wconversion -Winline -g -O2 -nostdlib -nostdinc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Winline -g -O2 -nostdlib -nostdinc")


##
##  Set up the linker
##
##  Note: Link crti, crtbegin, crtend and crtn from GCC for global
##        instantiation of classes.
##
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} -static -N -L${LIBGCCDIR} -e_start <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Keep the order of the object files and libraries
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} -static -N /usr/lib/crti.o ${LIBGCCDIR}/crtbegin.o -L${LIBGCCDIR} <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${LIBGCCDIR}/crtend.o /usr/lib/crtn.o")


##
## Generate boot floppy image
##
find_program(MKDIR mkdir)
execute_process(COMMAND ${MKDIR} ${CMAKE_BINARY_DIR}/bin ERROR_QUIET)
add_custom_target(bootdisk sh
    ${CMAKE_SOURCE_DIR}/Tools/Scripts/makediskimage.sh
    ${CMAKE_SOURCE_DIR}/Tools/Scripts ${CMAKE_BINARY_DIR}/bin)

exec_program(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_SOURCE_DIR}/Tools/Scripts/ext2/mbr.cc -o ${CMAKE_BINARY_DIR}/mbr)
find_program(MBRTOOL    ${CMAKE_BINARY_DIR}/mbr)

##
## Generate the RAM disk
##
set(RAMDISK         ramdisk)
add_custom_target(ramdisk_pre COMMAND ${MKEMPTY} ${RAMDISK})
add_custom_target(ramdisk)


set(ROOT_DISK       disk0.raw)
set(ROOT_DISK_SIZE  300M)       # 300MB
set(ROOT_FS         fs0.img)
set(FS_SIZE         614275)     # sectors

set(VMDISK0         hd0.vmdk)
set(QDISK0          hda.img)

set(BOOT_DIR        boot)
set(SYSTEM_DIR      system)
set(APP_DIR         bin)
set(HOME_DIR        home)
set(GRUB_DIR        boot/grub)

set(GRUB_SRC_DIR    ${CMAKE_SOURCE_DIR}/Tools/Grub)
set(BMAP_FILE       bmap)

##
## Generate the root disk
##
add_custom_target(rootdisk_pre
    COMMAND ${MKEMPTY} ${ROOT_DISK}
    # Create an empty raw disk image
    COMMAND ${QEMU_IMAGE} create ${ROOT_DISK} ${ROOT_DISK_SIZE}
    # Set up MBR
    COMMAND ${MBRTOOL} ${ROOT_DISK}
    COMMAND ${MKEMPTY} ${ROOT_FS}
    # Create an empty partition image
    COMMAND ${DD} if=/dev/zero of=${ROOT_FS} bs=512 count=${FS_SIZE}
    # Format the partition
    COMMAND ${MKE2FS} -q -F ${ROOT_FS}
    # Make directories
    COMMAND ${E2MKDIR} ${ROOT_FS}:${BOOT_DIR}
    COMMAND ${E2MKDIR} ${ROOT_FS}:${SYSTEM_DIR}
    COMMAND ${E2MKDIR} ${ROOT_FS}:${APP_DIR}
    COMMAND ${E2MKDIR} ${ROOT_FS}:${HOME_DIR}
    COMMAND ${E2CP} ${CMAKE_SOURCE_DIR}/Tools/Scripts/sound.bin ${ROOT_FS}:${HOME_DIR})

add_custom_target(rootdisk_post 
    COMMAND ${E2CP} ${RAMDISK} ${ROOT_FS}:${BOOT_DIR}
    # Install GRUB related files
    COMMAND ${E2MKDIR} ${ROOT_FS}:${GRUB_DIR}
    COMMAND ${E2CP} ${GRUB_SRC_DIR}/stage1 ${ROOT_FS}:${GRUB_DIR}/stage1
    COMMAND ${E2CP} ${GRUB_SRC_DIR}/e2fs_stage1_5 ${ROOT_FS}:${GRUB_DIR}
    COMMAND ${E2CP} ${GRUB_SRC_DIR}/stage2 ${ROOT_FS}:${GRUB_DIR}/stage2
    COMMAND ${E2CP} ${GRUB_SRC_DIR}/menu.lst ${ROOT_FS}:${GRUB_DIR}/menu.lst
    # Copy the partition into the disk image
    COMMAND ${DD} if=${ROOT_FS} of=${ROOT_DISK} bs=512 seek=63 count=${FS_SIZE}
    # Install GRUB
    COMMAND ${GRUB_INSTALL} ${CMAKE_BINARY_DIR}/${ROOT_DISK})

add_dependencies(rootdisk_post ramdisk)
add_dependencies(rootdisk_post rootdisk_pre)

# Convert the raw to the vmware format
add_custom_target(vmware_disk
    COMMAND ${QEMU_IMAGE} convert ${ROOT_DISK} -O vmdk ${VMDISK0})
add_dependencies(vmware_disk rootdisk_post)

# Convert the raw to the qemu format
add_custom_target(qemu_disk
    COMMAND ${QEMU_IMAGE} convert ${ROOT_DISK} -O qcow2 ${QDISK0})
add_dependencies(qemu_disk rootdisk_post)


##
## Generate a user disk
## (unused since 2008-09-08)
##
set(USER_DISK       disk1.raw)
set(USER_DISK_SIZE  300M)
set(USER_FS         fs1.img)
set(USER_FS_SIZE    614275)
set(MEDIA_DIR       media)
set(MEDIA_FILE      file.mov)
set(MEDIA_FILE_SIZE 18000)
set(VMDISK1         hd1.vmdk)
set(QDISK1          hdc.img)

add_custom_target(userdisk_pre
    COMMAND ${QEMU_IMAGE} create ${USER_DISK} ${USER_DISK_SIZE}
    COMMAND ${MBRTOOL} ${USER_DISK}
    COMMAND ${DD} if=/dev/zero of=${USER_FS} bs=512 count=${USER_FS_SIZE}
    COMMAND ${MKE2FS} -q -F ${USER_FS}
    COMMAND ${E2MKDIR} ${USER_FS}:${MEDIA_DIR})

add_custom_target(userdisk_post
    COMMAND ${DD} if=/dev/urandom of=${MEDIA_FILE} bs=1024 count=${MEDIA_FILE_SIZE}
    COMMAND ${E2CP} ${MEDIA_FILE} ${USER_FS}:${MEDIA_DIR}
    COMMAND ${DD} if=${USER_FS} of=${USER_DISK} bs=512 seek=63 count=${USER_FS_SIZE}
    COMMAND ${QEMU_IMAGE} convert ${USER_DISK} -O vmdk ${VMDISK1})

#add_dependencies(userdisk_post userdisk_pre)

##
## VMware setup
##

set(VMWARE_DIR      ${CMAKE_SOURCE_DIR}/Tools/VMware)

# 'USER' replaces the variables in the configuration files
exec_program(${WHOAMI} OUTPUT_VARIABLE USER)
configure_file(${CMAKE_SOURCE_DIR}/Tools/VMware/conf.vmx
    ${CMAKE_BINARY_DIR}/conf.vmx)
configure_file(${CMAKE_SOURCE_DIR}/Tools/VMware/conf.vmxf
    ${CMAKE_BINARY_DIR}/conf.vmxf)
configure_file(${CMAKE_SOURCE_DIR}/Tools/VMware/conf.vmsd
    ${CMAKE_BINARY_DIR}/conf.vmsd)

add_custom_target(vmnet_setup
    COMMAND ${CP} ${VMWARE_DIR}/nvram.net ${CMAKE_BINARY_DIR}/nvram)

add_custom_target(vmdisk_setup
    COMMAND ${CP} ${VMWARE_DIR}/nvram.disk ${CMAKE_BINARY_DIR}/nvram)

# Netboot (PXE boot)
# NOTE: This runs VMware.
add_custom_target(vmnet
    COMMAND ${MAKE} uninstall
    COMMAND ${MAKE} install
    COMMAND ${VMWARE} -x -q conf.vmx)
add_dependencies(vmnet vmnet_setup)
add_dependencies(vmnet vmware_disk)

# Disk boot
# NOTE: This runs VMware.
add_custom_target(vmdisk
    COMMAND ${VMWARE} -x -q conf.vmx)
add_dependencies(vmdisk vmware_setup)
add_dependencies(vmdisk vmware_disk)


##
## QEMU setup
##
# NOTE: This runs QEMU.
add_custom_target(qemu
    COMMAND ${QEMU} -hda ${QDISK0} -boot c -serial stdio -soundhw sb16)
add_dependencies(qemu qemu_disk)

##
## QEMU w/floppy
##
add_custom_target(qemufd
    COMMAND ${QEMU} -fda ${QFD0} -boot a -serial stdio -soundhw sb16)
add_dependencies(qemu qemu_floppy)

##
## Target 'uninstall'
##
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Tools/CMake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

##
## Build subdirectories
##
add_subdirectory(Kernel)
add_subdirectory(Libraries)
add_subdirectory(Services)
add_subdirectory(Applications)
add_subdirectory(Boot)

