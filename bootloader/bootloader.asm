[bits 16]

%define DRIVE_NUM dl
%define HEAD_NUM dh
%define CYLINDER_NUM ch
%define SECTOR_NUM cl
%define NUM_OF_SECTORS al

KERNEL_ADDR equ 0x10000
KERNEL_START_ADDR equ 0x11000
%define KERNEL_SEG    (KERNEL_ADDR >> 4)
%define KERNEL_OFF    (KERNEL_ADDR & 0xf)
KERNEL_ADDR_2 equ 0x17E00
%define KERNEL_SEG_2    (KERNEL_ADDR_2 >> 4)
%define KERNEL_OFF_2    (KERNEL_ADDR_2 & 0xf)
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

section .text
global boot

start:
    jmp boot
    nop

os_name db "MVLIRA05"; OS Name

%ifdef FLOPPY
%include "floppy_biosparams.inc"
%else
%include "ata_biosparams.inc"
%endif

boot:
    cli
    cld

    mov [boot_drive], DRIVE_NUM

    mov bh, 0

    mov ah, 0
    mov DRIVE_NUM, [boot_drive]
    int 0x13
    jc boot

    in al, 0x92
    or al, 2
    out 0x92, al

    mov bp, 0x9000
    mov sp, bp

    call load_kernel

    call getxy

    call switch_to_pm

getxy:
    push dx

    mov ah, 0x03
    mov bh, 0x0
    int 0x10

    mov [curr_y], dh
    mov [curr_x], dl

    pop dx
    ret

putchar:
    mov ah, 0x0E
    int 0x10

    ret

puts:
    .loop:
        lodsb
        or al, al
        jz .done

        mov cx, 1
        call putchar
        jmp .loop

    .done:
        ret

memdump:
    .loop:
        lodsb
        cmp cx, 0
        je .done

        dec cx
        call putchar
        jmp .loop

    .done:
        ret

disk_reset:
    mov ah, 0x00
    int 0x13
    ret

disk_load:
    push ax

    mov DRIVE_NUM, [boot_drive]
    call disk_reset

    mov ah, 0x02
    mov DRIVE_NUM, [boot_drive]

    int 0x13
    mov dh, al
    jc disk_error

    pop ax
    cmp dh, al
    jne disk_error

    ret

disk_error:
    mov si, disk_error_msg
    call puts
    jmp $

[bits 32]

gdt_start:
    gdt_null:
        dd 0x0
        dd 0x0

    gdt_code:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 10011010b
        db 11001111b
        db 0x0

    gdt_data:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 10010010b
        db 11001111b
        db 0x0
gdt_end:

gdt:
    dw gdt_end - gdt_start - 1
    dd gdt_start

%ifdef FLOPPY
%include "floppy_loadkernel.inc"
%else
%include "ata_loadkernel.inc"
%endif

switch_to_pm:
    cli
    lgdt [gdt]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

[bits 32]

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call begin_pm

[bits 32]

begin_pm:
    pop ax
    mov dx, 0
    mov dl, [boot_drive]

    shl edx, 16

    mov dh, [curr_y]
    mov dl, [curr_x]

    jmp CODE_SEG:KERNEL_START_ADDR

    hlt

;section .data
disk_error_msg db "Disk read error", 0xD, 0xA, 0x0
user_data dw 0
boot_drive db 0
curr_y db 0
curr_x db 0

times 446 - ($-$$) db 0

PARTITION1
PARTITION2
PARTITION3
PARTITION4

dw 0xAA55
