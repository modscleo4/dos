global syscall_handler

section .text
syscall_handler:
    extern run_syscall
    extern __syscall_ret

    push byte 1
    push byte 0

    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call run_syscall
    add esp, 4

    mov [__syscall_ret], eax

    pop gs
    pop fs
    pop es
    pop ds
    popad

    add esp, 8

    mov eax, [__syscall_ret]

    iret
