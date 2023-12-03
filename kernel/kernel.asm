[bits 32]

global _start
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

section .text.setup

_start:
    mov esp, proto_stack
    push ebx
    push eax

    mov eax, 0x0 ; Counter
    mov ebx, 0x0 ; Real address
    .fill_id_table:
        mov ecx, ebx
        or ecx, 3
        mov [table_0 + eax * 4], ecx
        invlpg [table_0 + eax * 4]
        add ebx, 4096
        inc eax
        cmp eax, 1024
        jne .fill_id_table

    mov eax, 0x0 ; Counter
    mov ebx, 0x000000 ; Real address
    .fill_kernel_table:
        mov ecx, ebx
        or ecx, 3
        mov [table_768 + eax * 4], ecx
        invlpg [table_768 + eax * 4]
        add ebx, 4096
        inc eax
        cmp eax, 1024
        jne .fill_kernel_table

enable_paging:
    mov ebx, table_0
    or ebx, 3
    mov [page_directory + 0], ebx

    mov ebx, table_768
    or ebx, 3
    mov [page_directory + 768 * 4], ebx

    mov eax, page_directory
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    pop eax
    pop ebx

    jmp stublet

section .text

stublet:
    extern kernel_main
    extern _esp

    mov esp, sys_stack
    mov ebp, esp
    mov [_esp], esp

    push 0
    popf

    push ebx
    push eax
    mov ebp, 0
    call kernel_main
    cli
    hlt
    jmp $

switch_ring3:
    extern __ring3_addr
    extern set_kernel_stack

    cli

    push esp
    call set_kernel_stack
    add esp, 4

    mov eax, [esp + 8]
    mov [__ring3_addr], eax
    mov esp, [esp + 12]

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
    push eax

    ;mov eax, [esp + 4]
    ;mov cr3, eax

    iret
    ret

section .bss

sys_stack_bottom:
    resb 0x8000
sys_stack:

section .bss.setup

page_directory:
    resd 1024
table_0:
    resd 1024
table_768:
    resd 1024
proto_stack_bottom:
    resb 0x100
proto_stack:
