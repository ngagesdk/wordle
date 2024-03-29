cmake_minimum_required(VERSION 3.00)

# Use CMake or Visual Studio to enable these settings.
option(GENERATE_SIS          "Generate Symbian SIS installer" OFF)
option(BUILD_ON_ALT_PLATFORM "Build on alternate platform"    OFF)

if(BUILD_ON_ALT_PLATFORM)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeLists.txt)
    return()
elseif(DEFINED ENV{NGAGESDK})
    set(NGAGESDK $ENV{NGAGESDK})
    set(CMAKE_TOOLCHAIN_FILE ${NGAGESDK}/cmake/ngage-toolchain.cmake)
else()
    message(FATAL_ERROR "The environment variable NGAGESDK needs to be defined.")
endif()

project(wordle C CXX)

set(UID1    0x1000007a) # KExecutableImageUidValue, e32uid.h
set(UID2    0x100039ce) # KAppUidValue16, apadef.h
set(UID3    0x1000abcd) # game.exe UID
set(APP_UID 0x1000dcba) # wordle.app UID

set(GCC_COMN_DEFS -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS -DNDEBUG -D_UNICODE)
set(GCC_DEFS      ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(game_libs
    ${EXTRA_LIB}/libSDL.a
    ${EPOC_PLATFORM}/gcc/lib/gcc-lib/arm-epoc-pe/2.9-psion-98r2/libgcc.a
    ${EPOC_LIB}/egcc.lib
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/estlib.lib
    ${EPOC_LIB}/ws32.lib
    ${EPOC_LIB}/hal.lib
    ${EPOC_LIB}/efsrv.lib
    ${EPOC_LIB}/scdv.lib
    ${EPOC_LIB}/gdi.lib)

set(wordle_libs
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/apparc.lib
    ${EPOC_LIB}/cone.lib
    ${EPOC_LIB}/eikcore.lib
    ${EPOC_LIB}/avkon.lib)

set(SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")

set(game_sources
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/game.c"
    "${SRC_DIR}/osd.c"
    "${SRC_DIR}/pfs.c"
    "${SRC_DIR}/utils.c"
    "${SRC_DIR}/wordlist_de.c"
    "${SRC_DIR}/wordlist_en.c"
    "${SRC_DIR}/wordlist_fi.c"
    "${SRC_DIR}/wordlist_ru.c"
    "${SRC_DIR}/wordlist_utils.c")

set(game_resources
    "tiles.png"
    "font.png"
    "disclaimer.png")

set(wordle_sources
    "${SRC_DIR}/wordle.cpp"
    "${SRC_DIR}/wordle_application.cpp"
    "${SRC_DIR}/wordle_appui.cpp"
    "${SRC_DIR}/wordle_appview.cpp"
    "${SRC_DIR}/wordle_document.cpp")

add_library(game   STATIC ${game_sources})
add_library(wordle STATIC ${wordle_sources})

build_exe(game exe   ${UID1} ${UID2} ${UID3}    "${game_libs}")
build_dll(wordle app ${UID1} ${UID2} ${APP_UID} "${wordle_libs}")

pack_assets(${RESOURCE_DIR} "${game_resources}")
build_aif(${RESOURCE_DIR} wordle ${APP_UID})
build_resource(${SRC_DIR} wordle "")

if(GENERATE_SIS)
  build_sis(${RESOURCE_DIR} wordle)

  add_dependencies(
    wordle.sis
    game.exe
    wordle.aif
    wordle.app
    wordle.rsc)
endif()

add_dependencies(
    game.exe
    game)

target_compile_definitions(
    game
    PUBLIC
    __EXE__
    FUNCTION_NAME=__FUNCTION__
    STBI_NO_THREAD_LOCALS
    ${GCC_DEFS}
    UID1=${UID1}
    UID2=${UID2}
    UID3=${UID3})

target_compile_options(
    game
    PUBLIC
    -Wall
    -O3)

target_include_directories(
    game
    PUBLIC
    ${SRC_DIR}
    ${SDL_INC_DIR})

add_dependencies(
    wordle.app
    wordle)

target_compile_definitions(
    wordle
    PUBLIC
    __DLL__
    ${GCC_DEFS}
    UID1=${UID1}
    UID2=${UID2}
    UID3=${APP_UID})

target_compile_options(
    wordle
    PUBLIC
    -O3)

target_include_directories(
    wordle
    PUBLIC
    ${SRC_DIR})
