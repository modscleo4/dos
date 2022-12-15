%ifndef ATA_PART_ASM
%define ATA_PART_ASM

%macro PARTITION1 0
db 0x80 ; Bootable

; CHS start
db 0x00
db 0x02
db 0x00

db 0x7F ; Partittion type

; CHS end
db 0x02
db 0x09
db 0x01

dd 0x00000001 ; LBA start
dd 0x00000200 ; Number of sectors
%endmacro

%macro PARTITION2 0
db 0x00 ; Bootable

; CHS start
db 0x02
db 0x0A
db 0x01

db 0x83 ; Partittion type

; CHS end
db 0x0A
db 0x08
db 0x02

dd 0x00000201 ; LBA start
dd 0x00007DFF ; Number of sectors
%endmacro

%macro PARTITION3 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%macro PARTITION4 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%endif
