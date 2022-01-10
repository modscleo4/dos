[bits 16]

%define DRIVE_NUM dl
%define HEAD_NUM dh
%define CYLINDER_NUM ch
%define SECTOR_NUM cl
%define NUM_OF_SECTORS al

KERNEL_ADDR equ 0x10000
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

    call check_a20
    cmp ax, 1
    jne a20_not_enabled

    call enable_a20
    cmp ax, 1
    jne a20_not_enabled

a20_enabled:

    call load_kernel

    call getxy

    call switch_to_pm

a20_not_enabled:
    hlt

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

check_a20:
    pushf
    push ds
    push es
    push di
    push si

    cli

    xor ax, ax ; ax = 0
    mov es, ax

    not ax ; ax = 0xFFFF
    mov ds, ax

    mov di, 0x0500
    mov si, 0x0510

    mov al, byte [es:di]
    push ax

    mov al, byte [ds:si]
    push ax

    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF

    cmp byte [es:di], 0xFF

    pop ax
    mov byte [ds:si], al

    pop ax
    mov byte [es:di], al

    mov ax, 0
    je check_a20__exit

    mov ax, 1

check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret

enable_a20:
    mov     ax, 0x2403
    int     0x15
    jb      a20_ns
    cmp     ah, 0
    jnz     a20_ns

    mov     ax, 0x2402
    int     0x15
    jb      a20_failed
    cmp     ah, 0
    jnz     a20_failed

    cmp     al, 1
    jz      a20_activated

    mov     ax, 0x2401
    int     0x15
    jb      a20_failed
    cmp     ah, 0
    jnz     a20_failed

a20_activated:
    mov ax, 1
    ret

a20_ns:
    mov ax, 0
    ret

a20_failed:
    hlt

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

    mov bx, KERNEL_SEG
    mov es, bx
    mov bx, KERNEL_OFF
    mov NUM_OF_SECTORS, 63
    mov CYLINDER_NUM, 1 ; Sector 33 + 0x10 (skip ELF header)
    mov HEAD_NUM, 0
    mov SECTOR_NUM, 6
    call disk_load

    mov bx, KERNEL_SEG_2
    mov es, bx
    mov bx, KERNEL_OFF_2
    mov NUM_OF_SECTORS, 63
    mov CYLINDER_NUM, 2 ; Sector 33 + 0x10 (skip ELF header) + 63 (previously loaded)
    mov HEAD_NUM, 1
    mov SECTOR_NUM, 15
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

    call begin_pm

[bits 32]

begin_pm:
    pop ax
    mov dh, [boot_drive]

    shl edx, 16

    mov dh, [curr_y]
    mov dl, [curr_x]

    jmp CODE_SEG:KERNEL_ADDR

    hlt

;section .data
disk_error_msg db "Disk read error", 0xD, 0xA, 0x0
user_data dw 0
boot_drive db 0
curr_y db 0
curr_x db 0

times 510 - ($-$$) db 0

dw 0xAA55
