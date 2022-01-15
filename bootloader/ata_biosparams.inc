%ifndef ATA_ASM
%define ATA_ASM

bios_param:
    param_bytes_per_sector dw 0x0200; Bytes per Sector
    param_sectors_per_cluster db 0x01; Sectors per Cluster
    param_reserved_sectors dw 0x0001; Reserved Sectors
    param_number_of_fat db 0x02; Number of file allocation tables
    param_rootdir_entries dw 0x00E0; Root Directory Entries
    param_small_sectors dw 0x0B40; Small Sectors
    param_media_type db 0xF0; Media Type
    param_sectors_per_fat dw 0x0009; Sectors per file allocation table
    param_sectors_per_track dw 0x0020; Sectors per Track
    param_number_of_heads dw 0x0002; Number of Heads
    param_hidden_sectors dd 0x00000000; Hidden Sectors
    param_large_sectors dd 0x00000000; Large Sectors

ext_bios_param:
    param_disk_number db 0x00; Physical Disk Number
    param_current_head db 0x00; Current Head
    param_signature db 0x29; Signature
    param_serial_number dd 0xCE134630; Volume Serial Number
    param_volume_label db "ATA_MARCOS", 0x20; Volume Label
    param_filesystem db "FAT12", 0x20, 0x20, 0x20; System ID

%macro PARTITION1 0
db 0x00, 0x00, 0x02, 0x00, 0x01, 0x41, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x0F, 0x00, 0x00
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
