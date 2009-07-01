
include_directories(
    ${CMAKE_SOURCE_DIR}/Libraries/System/include
    ${CMAKE_SOURCE_DIR}/Libraries/Arc/include
    ${CMAKE_SOURCE_DIR}/Libraries/C/include)
set(LIB_NAME x86emu)
add_definitions(-DNO_SYS_HEADERS)
include(${CMAKE_SOURCE_DIR}/Tools/CMake/Library.cmake)

