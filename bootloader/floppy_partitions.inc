%ifndef FLOPPY_PART_ASM
%define FLOPPY_PART_ASM

%macro PARTITION1 0
db 0x80 ; Bootable

; CHS start
db 0x00
db 0x02
db 0x00

db 0x7F ; Partition type

; CHS end
db 0x01
db 0x03
db 0x05

dd 0x00000001 ; LBA start
dd 0x00000200 ; Number of sectors
%endmacro

%macro PARTITION2 0
db 0x00 ; Bootable

; CHS start
db 0x01
db 0x04
db 0x05

db 0x01 ; Partition type

; CHS end
db 0x2D
db 0x2D
db 0x00

dd 0x00000201 ; LBA start
dd 0x0000093F ; Number of sectors
%endmacro

%macro PARTITION3 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%macro PARTITION4 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%endif
