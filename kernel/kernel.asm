[bits 32]

global kernel_start

global _sys_stack

section .text

kernel_start:
    mov esp, _sys_stack
    jmp stublet

stublet:
    extern kernel_main
    call kernel_main
    jmp $

section .bss
    resb 8192
_sys_stack:
