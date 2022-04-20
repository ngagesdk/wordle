cmake_minimum_required(VERSION 3.0)

project(wordle)

find_package(SDL2 REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")

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
    ${SDL2_LIBRARIES})

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
    HAVE_CTYPE_H
    SDL_MAIN_HANDLED)
