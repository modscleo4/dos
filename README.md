# DOS
A Simple x86 Operating System written in C.

## What's working
### Grub2
The Kernel is loaded by Grub2. The boot configuration is located in `rootfs/boot/grub/grub.cfg`.

### Multiboot 2
The Kernel is loaded by Grub2 using the Multiboot 2 protocol. The Kernel is loaded at `0x100000` and is loaded in 32-bit mode.

### MBR Parsing
The Kernel parses the MBR and finds its own partition. It does not support extended partitions. Then it setups the filesystem and mounts it.

### FAT12/16
The Kernel supports FAT12 and FAT16. It can read files and directories. The Kernel cannot write to FAT12/16.

### Ext2
The Kernel supports Ext2. It can read files and directories. The Kernel cannot write to Ext2.

### Syscalls
The Kernel supports syscalls via the `int 0x80` instruction. Currently the following syscalls are supported:
- `sys_write`
- `sys_read`
- `sys_open`
- `sys_close`
- `sys_stat`
- `sys_getpid`
- `sys_fork`
- `sys_execve`
- `sys_exit`

### ELF parsing
The Kernel can parse ELF files and execute them. It can also load debug symbols from itself.

### Kernel Modules
Currently the Kernel is built as a single binary. In the future it will be split into modules. The Kernel will load the modules from the filesystem.

### Networking
The Kernel supports the Intel 82540EM series of network cards. It can send and receive packets. The following protocols are supported:
- ARP
- IPv4
- UDP
- TCP
- DHCP
- DNS

### Libc
The Kernel is built on top of a primitive libc. It is located in `kernel/libc`. There's also a `system/libc` which is used by the userspace programs and in the future will generate a shared library.

## Goals
- [x] Grub2
- [x] Multiboot 2
- [x] MBR Parsing
- [x] FAT12/16
- [x] Ext2
- [x] ISO9660
- [ ] x86_64
- [x] Syscalls
- [x] Multitasking
- [x] Virtual Memory
- [x] ELF parsing
- [] Dynamic Linking
- [x] Kernel Modules
- [x] ACPI
- [ ] ACPICA
- [x] PCI
- [x] ATA
- [x] ATAPI
- [x] Networking
- [ ] Virtual File System
- [ ] Init System
- [ ] Shell
- [x] Libc
