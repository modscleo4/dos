[bits 32]

global kernel_start
global switch_ring3

global sys_stack

section .text

kernel_start:
    mov esp, sys_stack
    push esp
    jmp stublet

stublet:
    extern kernel_main
    call kernel_main
    cli
    hlt
    jmp $

switch_ring3:
    extern start_shell
    mov ax,0x23
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    mov eax,esp
    push 0x23
    push eax
    pushf
    push 0x1B
    push start_shell
    iret

section .bss
    resb 8192
sys_stack:
