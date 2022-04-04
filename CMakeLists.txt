cmake_minimum_required(VERSION 3.00)

if(DEFINED ENV{NGAGESDK})
    SET(NGAGESDK $ENV{NGAGESDK})
    set(CMAKE_TOOLCHAIN_FILE ${NGAGESDK}/cmake/ngage-toolchain.cmake)
else()
    message(FATAL_ERROR "The environment variable NGAGESDK needs to be defined.")
endif()

project(wordle C CXX)

include(SDL)
include(dbgprint)

# Use CMake or Visual Studio to enable these settings.
option(INSTALL_EKA2L1 "Install app for EKA2L1" OFF)

set(UID1 0x1000007a) # KExecutableImageUidValue, e32uid.h
set(UID2 0x100039ce) # KAppUidValue16, apadef.h
set(UID3 0x1000abcd) # wordle UID

set(GCC_COMN_DEFS -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS -DNDEBUG -D_UNICODE)
set(GCC_DEFS      ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(wordle_libs
    ${CMAKE_CURRENT_BINARY_DIR}/libSDL.a
    ${CMAKE_CURRENT_BINARY_DIR}/libdbgprint.a
    ${EPOC_PLATFORM}/gcc/lib/gcc-lib/arm-epoc-pe/2.9-psion-98r2/libgcc.a
    ${EPOC_LIB}/egcc.lib
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/estlib.lib
    ${EPOC_LIB}/ws32.lib
    ${EPOC_LIB}/hal.lib
    ${EPOC_LIB}/efsrv.lib
    ${EPOC_LIB}/scdv.lib
    ${EPOC_LIB}/gdi.lib)

set(SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")

set(wordle_sources
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/game.c"
    "${SRC_DIR}/pfs.c"
    "${SRC_DIR}/utils.c"
    "${SRC_DIR}/wordlist.c")

set(wordle_resources
    "tiles.bmp")

add_library(wordle STATIC ${wordle_sources})
build_exe(wordle exe ${UID1} ${UID2} ${UID3} "${wordle_libs}")
pack_assets(${RESOURCE_DIR} "${wordle_resources}")
if(INSTALL_EKA2L1)
    copy_file(wordle.exe ${CMAKE_CURRENT_BINARY_DIR} ${EKA2L1_E_DRIVE}/System/Apps/wordle wordle.exe)
    copy_file(data.pfs   ${RESOURCE_DIR}             ${EKA2L1_E_DRIVE}/System/Apps/wordle data.pfs)
endif()

add_dependencies(
    wordle.exe
    wordle)

add_dependencies(
    wordle
    SDL)

target_compile_definitions(
    wordle
    PUBLIC
    __EXE__
    FUNCTION_NAME=__FUNCTION__
    ${GCC_DEFS}
    UID1=${UID1}
    UID2=${UID2}
    UID3=${UID3})

target_compile_options(
    wordle
    PUBLIC
    -O3)

target_include_directories(
    wordle
    PUBLIC
    ${SRC_DIR}
    ${SDL_INC_DIR})
