cmake_minimum_required(VERSION 3.16)
project(kernel)

set(CMAKE_CXX_STANDARD 99)

set(SOURCE_FILES
        shell/shell.c
        shell/shell.h)

add_executable(system ${SOURCE_FILES})
