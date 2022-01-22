[bits 16]

os_name db "MVLIRA05"; OS Name

%ifdef FLOPPY
%include "floppy_biosparams.inc"
%else
%include "ata_biosparams.inc"
%endif

times 28 db 0x0

dw 0x8000 ; Address
GRUB_BOOT_SECTOR ; LBA
db 0x00
db 0x00
db 0x00
db 0x00
