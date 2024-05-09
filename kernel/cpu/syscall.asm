global syscall_handler

section .text
syscall_handler:
    extern run_syscall
    extern __syscall_ret
    extern mmu_load_kernel_pdt
    extern process_unload
    extern process_reload
    extern process_switch

    cli
    push byte 1
    push byte 0

    pushad
    push ds
    push es
    push fs
    push gs

    mov eax, cr0
    push eax
    mov eax, cr2
    push eax
    mov eax, cr3
    push eax
    mov eax, cr4
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    ;call mmu_load_kernel_pdt
    call process_unload; save current process state
    call run_syscall

    mov [__syscall_ret], eax

    call process_reload; reload new process state

    ;push 0; PID
    ;call process_switch
    ;add esp, 4

    add esp, 4

    add esp, 16

    pop gs
    pop fs
    pop es
    pop ds
    popad

    add esp, 8

    mov eax, [__syscall_ret]

    iret
