[bits 16]

%define DRIVE_NUM dl
%define HEAD_NUM dh
%define CYLINDER_NUM ch
%define SECTOR_NUM cl
%define NUM_OF_SECTORS al

section .text
global boot

start:
    jmp boot
    nop

%ifdef FLOPPY
%include "floppy_partitions.inc"
%elifdef ATA
%include "ata_partitions.inc"
%elifdef ATAEXT2
%include "ata_ext2_partitions.inc"
%endif

boot:
    cli
    cld

    xor ax, ax ; mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, ax

    .copy_lower:
        mov cx, 0x1000
        mov si, 0x7C00
        mov di, 0x0600
        rep movsb

    jmp 0:low_start

low_start:
    sti

    mov [boot_drive], DRIVE_NUM

    mov bh, 0

    mov ah, 0
    mov DRIVE_NUM, [boot_drive]
    int 0x13
    jc reset_fail

    mov bp, 0x9000
    mov sp, bp

    call find_bootable_partition

    mov si, no_bootable_partition
    call puts
    hlt

reset_fail:
    hlt

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
    jc disk_error

    mov dh, al
    pop ax
    cmp dh, al
    jne disk_error

    ret

disk_error:
    mov si, disk_error_msg
    call puts
    jmp $

disk_error_sectors:
    mov si, disk_error_sectors_msg
    call puts
    jmp $

find_bootable_partition:
    mov bx, 0x01BE
    add bx, 0x0600
    .read_mbr:
        .no_bootable:
            add bx, 0x0010
            cmp bx, 0x01FE
            je .no_bootable_partition_found

        cmp byte [bx], 0x80
        jne .no_bootable

        mov si, loading_msg
        call puts

        ; Bootable partition found
        ; Read first sector of partition
        mov [_bx], bx
        mov HEAD_NUM, [bx + 0x1]
        mov SECTOR_NUM, [bx + 0x2]
        mov CYLINDER_NUM, [bx + 0x3]
        mov NUM_OF_SECTORS, 1
        mov bx, 0
        mov es, bx
        mov bx, 0x7C00
        call disk_load

        cmp word [0x7C00 + 0x1FE], 0xAA55
        jne .invalid_mbr

        jmp 0x0:0x7C00

        .invalid_mbr:
            mov bx, [_bx]
            mov si, invalid_mbr_msg
            call puts
            jmp .no_bootable

    .no_bootable_partition_found:

    ret

loading_msg db "Loading...", 0xD, 0xA, 0x00
disk_error_msg db "Disk read error", 0xD, 0xA, 0x0
disk_error_sectors_msg db "Disk read error: Sectors read not equal to sectors requested", 0xD, 0xA, 0x0
no_bootable_partition db "No bootable partition found", 0xD, 0xA, 0x0
invalid_mbr_msg db "Invalid MBR", 0xD, 0xA, 0x0
user_data dw 0
boot_drive db 0
curr_y db 0
curr_x db 0
_bx dw 0

times 440 - ($-$$) db 0
dd 0x01020304
dw 0x0000

PARTITION1
PARTITION2
PARTITION3
PARTITION4

dw 0xAA55
