cmake_minimum_required(VERSION 3.16)
project(system)

set(CMAKE_CXX_STANDARD 99)

set(SOURCE_FILES
        libc/stdio.c libc/include/stdio.h
        libc/string.c libc/include/string.h
        libc/stdlib.c libc/include/stdlib.h
        libc/ctype.c libc/include/ctype.h
        libc/include/stddef.h
        libc/include/stdint.h
        libc/include/stdarg.h
        libc/math.c libc/include/math.h
        libc/unistd.c libc/include/unistd.h
        shell/shell.c
        shell/shell.h)

add_executable(system ${SOURCE_FILES})
