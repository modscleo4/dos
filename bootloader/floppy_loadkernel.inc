[bits 16]
load_kernel:
    ; We will assume that the Kernel is the first file in the disk

    mov bx, KERNEL_SEG
    mov es, bx
    mov bx, KERNEL_OFF
    mov NUM_OF_SECTORS, 63
    mov CYLINDER_NUM, 0 ; Sector 33
    mov HEAD_NUM, 1
    mov SECTOR_NUM, 16
    call disk_load

    mov bx, KERNEL_SEG_2
    mov es, bx
    mov bx, KERNEL_OFF_2
    mov NUM_OF_SECTORS, 63
    mov CYLINDER_NUM, 2 ; Sector 33 + 63 (previously loaded)
    mov HEAD_NUM, 1
    mov SECTOR_NUM, 7
    call disk_load

    ret
