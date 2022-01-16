[bits 16]
load_kernel:
    ; We will assume that the Kernel is the first file in the disk

    mov ah, 8
    mov dl, [boot_drive]
    int 0x13

    mov bx, KERNEL_SEG
    mov es, bx
    mov bx, KERNEL_OFF
    mov NUM_OF_SECTORS, 127
    mov CYLINDER_NUM, 0 ; Sector (1+20*2+56)=97
    mov HEAD_NUM, 3
    mov SECTOR_NUM, 2
    call disk_load

    ret
