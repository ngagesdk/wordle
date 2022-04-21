cmake_minimum_required(VERSION 3.0)

project(wordle)

find_package(SDL2 REQUIRED)

set(SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../res")

if(EMSCRIPTEN)
    configure_file(${RESOURCE_DIR}/data.pfs ${CMAKE_CURRENT_BINARY_DIR}/data.pfs COPYONLY)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}   -s USE_SDL=2")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --preload-file data.pfs")
endif()

include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${SRC_DIR})

set(wordle_sources
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

add_executable(
    wordle
    ${wordle_sources})

target_link_libraries(
    wordle
    ${SDL2_LIBRARIES}
    ${SDL_LIBS})

target_link_options(
    wordle
    PUBLIC
    -static-libgcc
    -static-libstdc++)

if(UNIX)
    target_link_libraries(wordle m)
endif(UNIX)

target_compile_options(
    wordle
    PUBLIC
    -Wall
    -Wextra
    -Wpedantic)

target_compile_definitions(
    wordle
    PUBLIC
    SDL_MAIN_HANDLED)
