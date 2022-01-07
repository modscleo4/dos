[bits 32]

global kernel_start
global switch_ring3

global sys_stack

section .text

kernel_start:
    mov esp, sys_stack
    jmp stublet

stublet:
    extern kernel_main
    push esp
    call kernel_main
    cli
    hlt
    jmp $

switch_ring3:
    extern __ring3_addr

    mov eax, [esp + 4]
    mov [__ring3_addr], eax

    mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax ; SS is handled by iret

	; set up the stack frame iret expects
	mov eax, esp
	push (4 * 8) | 3 ; data selector
	push eax ; current esp
	pushf ; eflags
	push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)

    mov eax, [__ring3_addr]
    ;mov eax, [esp + 16]
    push eax
    iret
    ret

section .bss

sys_stack_start:
    resb 8192
sys_stack:
