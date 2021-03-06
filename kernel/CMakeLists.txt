cmake_minimum_required(VERSION 3.16)
project(kernel)

set(CMAKE_C_STANDARD 99)

include_directories(SYSTEM ./libc/include)

set(SOURCE_FILES
        kernel.c kernel.h
        bits.c bits.h
        cpu/idt.c cpu/idt.h
        cpu/irq.c cpu/irq.h
        cpu/gdt.c cpu/gdt.h
        cpu/isr.c cpu/isr.h
        cpu/pic.c cpu/pic.h
        cpu/syscall.c cpu/syscall.h
        cpu/system.h
        modules/timer.c modules/timer.h
        modules/kblayout/us.h
        libc/stdio.c libc/include/stdio.h
        libc/string.c libc/include/string.h
        libc/stdlib.c libc/include/stdlib.h
        libc/ctype.c libc/include/ctype.h
        libc/include/stddef.h
        libc/include/stdint.h
        libc/include/stdarg.h
        libc/math.c libc/include/math.h
        drivers/fat.c drivers/fat.h
        drivers/screen.c drivers/screen.h
        drivers/keyboard.c drivers/keyboard.h
        drivers/floppy.c drivers/floppy.h)

add_executable(kernel ${SOURCE_FILES})
