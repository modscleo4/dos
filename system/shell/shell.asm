[bits 32]

global init

section .text

init:
    extern main
    call main
    ret
