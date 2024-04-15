cmake_minimum_required(VERSION 3.27)
project(1brc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-ansi -pg -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

add_executable(1brc main.c
        include/zfile.h
        src/zfile.c
        include/structures.h
        src/utils.c)
TARGET_LINK_LIBRARIES(1brc ${GLIB_LIBRARIES})
