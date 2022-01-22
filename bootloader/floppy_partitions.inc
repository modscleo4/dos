%ifndef FLOPPY_PART_ASM
%define FLOPPY_PART_ASM

%macro PARTITION1 0
db 0x80

db 0x00
db 0x02
db 0x00

db 0x01

db 0x2D
db 0x2D
db 0x00

dd 0x00000001
dd 0x00000B3F
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
