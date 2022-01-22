%ifndef ATA_PART_ASM
%define ATA_PART_ASM

%macro PARTITION1 0
db 0x80

db 0x00
db 0x02
db 0x00

db 0x04

db 0x0A
db 0x08
db 0x02

dd 0x00000001
dd 0x00007FFF
%endmacro

%macro PARTITION2 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%macro PARTITION3 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%macro PARTITION4 0
db 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
%endmacro

%endif
