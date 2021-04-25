[bits 16]

%define DRIVE_NUM dl
%define HEAD_NUM dh
%define TRACK_NUM ch
%define SECTOR_NUM cl
%define NUM_OF_SECTORS al

KERNEL_OFFSET equ 0x1000
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

section .text
global boot

start:
    jmp boot
    nop

os_name db "MVLIRA05"; OS Name

bios_param:
    param_bytes_per_sector dw 0x0200; Bytes per Sector
    param_sectors_per_cluster db 0x01; Sectors per Cluster
    param_reserved_sectors dw 0x0001; Reserved Sectors
    param_number_of_fat db 0x02; Number of file allocation tables
    param_rootdir_entries dw 0x00E0; Root Directory Entries
    param_small_sectors dw 0x0B40; Small Sectors
    param_media_type db 0xF0; Media Type
    param_sectors_per_fat dw 0x0009; Sectors per file allocation table
    param_sectors_per_track dw 0x0012; Sectors per Track
    param_number_of_heads dw 0x0002; Number of Heads
    param_hidden_sectors dd 0x00000000; Hidden Sectors
    param_large_sectors dd 0x00000000; Large Sectors

ext_bios_param:
    param_disk_number db 0x00; Physical Disk Number
    param_current_head db 0x00; Current Head
    param_signature db 0x29; Signature
    param_serial_number dd 0xCE134630; Volume Serial Number
    param_volume_label db "MARCOSLIRA", 0x20; Volume Label
    param_filesystem db "FAT12", 0x20, 0x20, 0x20; System ID

boot:
    cli
    cld

    mov [boot_drive], DRIVE_NUM

    mov bh, 0

    mov ah, 0
    mov DRIVE_NUM, [boot_drive]
    int 0x13
    jc boot

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

disk_load:
    push dx

    mov DRIVE_NUM, [boot_drive]
    mov ah, 0x00
    int 0x13

    mov ah, 0x02
    mov DRIVE_NUM, [boot_drive]
    mov NUM_OF_SECTORS, dh
    mov TRACK_NUM, 1
    mov HEAD_NUM, 0
    mov SECTOR_NUM, 6

    int 0x13
    jc disk_error

    pop dx
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

[bits 16]

load_kernel:
    ; We will assume that the Kernel is the first file in the disk
    mov bx, KERNEL_OFFSET
    mov dh, 54
    call disk_load

    ret

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

    mov ebp, 0x90000
    mov esp, ebp

    call begin_pm

[bits 32]

begin_pm:
    pop ax
    mov dh, [boot_drive]

    shl edx, 16

    mov dh, [curr_y]
    mov dl, [curr_x]

    call KERNEL_OFFSET

    hlt

;section .data
disk_error_msg db "Disk read error", 0xD, 0xA, 0x0
user_data dw 0
boot_drive db 0
curr_y db 0
curr_x db 0

times 510 - ($-$$) db 0

dw 0xAA55
