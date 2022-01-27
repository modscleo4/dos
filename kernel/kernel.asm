[bits 32]

global kernel_start
global switch_ring3

global sys_stack

%include "multiboot2.inc"

section .multiboot2_header

multiboot_header:
    align 8

    dd MULTIBOOT2_HEADER_MAGIC
    dd GRUB_MULTIBOOT_ARCHITECTURE_I386
    dd multiboot_header_end - multiboot_header
    dd -(MULTIBOOT2_HEADER_MAGIC + GRUB_MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))
framebuffer_tag_start:
    align 8

    ;dw MULTIBOOT_HEADER_TAG_FRAMEBUFFER
    ;dw MULTIBOOT_HEADER_TAG_OPTIONAL
    ;dd framebuffer_tag_end - framebuffer_tag_start
    ;dd 800
    ;dd 600
    ;dd 32
framebuffer_tag_end:
    align 8

    dw MULTIBOOT_HEADER_TAG_END
    dw 0
    dd 8
multiboot_header_end:

section .text

kernel_start:
    jmp stublet

stublet:
    extern kernel_main
    extern _esp

    mov esp, sys_stack
    mov [_esp], esp

    push 0
    popf

    push ebx
    push eax
    call kernel_main
    cli
    hlt
    jmp $

switch_ring3:
    extern __ring3_addr

    cli
    mov eax, [esp + 4]
    mov [__ring3_addr], eax
    mov esp, [esp + 8]

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
    pop eax
    or eax, 0x200 ; set IF
    push eax ; new eflags
	push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)

    mov eax, [__ring3_addr]
    push eax
    iret


section .bss

sys_stack_start:
    resb 16384
sys_stack:
