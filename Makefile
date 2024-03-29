BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/bootloader/bootloader
BIOSPARAMS=$(BUILD_DIR)/bootloader/biosparams
KERNEL=$(BUILD_DIR)/kernel/kernel.elf
SYSTEM_INIT=$(BUILD_DIR)/system/init/init.elf
FLOPPY_DISK_IMG=floppy_disk.img
ATA_DISK_IMG=ata_disk.img
ATA_DISK_EXT2_IMG=ata_disk_ext2.img
CDROM_ISO=cdrom.iso
VBOX_VDI=ata_disk.vdi
VMWARE_VMDK=ata_disk.vmdk
GRUB2_BOOT_IMG=/usr/lib/grub/i386-pc/boot.img
GRUB2_BOOT_ELTORITO=/usr/lib/grub/i386-pc/stage2_eltorito
GRUB2_CORE_IMG=grub2.img
FLOPPY_GRUB2_CORE_SECTOR=floppy_grub2_sector
ATA_GRUB2_CORE_SECTOR=ata_grub2_sector
ATA_EXT2_GRUB2_CORE_SECTOR=ata_ext2_grub2_sector

# Executables
BOCHS="/mnt/c/Program Files/Bochs-2.7/bochsdbg.exe"
QEMU="/mnt/c/Program Files/QEMU/qemu-system-x86_64.exe"
VBOXMANAGE="/mnt/c/Program Files/Oracle/VirtualBox/VBoxManage.exe"

all: dir disk

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: disk bootloader kernel system

bootloader_floppy:
	make -C bootloader clean
	make -C bootloader ARG1="-dFLOPPY"

bootloader_ata:
	make -C bootloader clean
	make -C bootloader ARG1="-dATA"

bootloader_ata_ext2:
	make -C bootloader clean
	make -C bootloader ARG1="-dATAEXT2"

kernel:
	make -C kernel

system:
	make -C system

grub2_ata:
	grub-mkimage -o $(GRUB2_CORE_IMG) -O i386-pc -p /boot/grub -c grub_ata.cfg configfile fat ext2 multiboot2 part_msdos ls nativedisk biosdisk normal help

grub2_floppy:
	grub-mkimage -o $(GRUB2_CORE_IMG) -O i386-pc -p /boot/grub -c grub_floppy.cfg configfile fat multiboot2 part_msdos ls nativedisk biosdisk normal help

rootfs: kernel system
	mkdir -p rootfs/boot
	cp $(KERNEL) rootfs
	cp $(SYSTEM_INIT) rootfs

floppy: bootloader_floppy rootfs grub2_floppy
	dd if=/dev/zero of=$(FLOPPY_DISK_IMG) bs=512 count=2880
	dd conv=notrunc if=$(BOOTLOADER) of=$(FLOPPY_DISK_IMG) bs=512 count=1 seek=0
	MTOOLSRC=./mtoolsrc mformat a:
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(FLOPPY_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(FLOPPY_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(FLOPPY_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(FLOPPY_DISK_IMG) bs=512 seek=2
	MTOOLSRC=./mtoolsrc mlabel -n a:MARCOSLIRA
	MTOOLSRC=./mtoolsrc mcopy -m -s rootfs/boot a:
	MTOOLSRC=./mtoolsrc mcopy -m rootfs/kernel.elf a:
	MTOOLSRC=./mtoolsrc mcopy -m rootfs/init.elf a:
	MTOOLSRC=./mtoolsrc mattrib +s a:/kernel.elf
	MTOOLSRC=./mtoolsrc mattrib +s a:/init.elf

ata: bootloader_ata rootfs grub2_ata
	dd if=/dev/zero of=$(ATA_DISK_IMG) bs=512 count=32768
	dd conv=notrunc if=$(BOOTLOADER) of=$(ATA_DISK_IMG) bs=512 count=1 seek=0
	MTOOLSRC=./mtoolsrc mformat c:
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(ATA_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(ATA_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(ATA_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(ATA_DISK_IMG) bs=512 seek=2
	MTOOLSRC=./mtoolsrc mlabel -n c:MARCOSLIRA
	MTOOLSRC=./mtoolsrc mcopy -m -s rootfs/boot c:
	MTOOLSRC=./mtoolsrc mcopy -m rootfs/kernel.elf c:
	MTOOLSRC=./mtoolsrc mcopy -m rootfs/init.elf c:
	MTOOLSRC=./mtoolsrc mattrib +s c:/kernel.elf
	MTOOLSRC=./mtoolsrc mattrib +s c:/init.elf

ataext2: bootloader_ata_ext2 rootfs grub2_ata
	dd if=/dev/zero of=$(ATA_DISK_IMG) bs=512 count=32768
	dd conv=notrunc if=$(BOOTLOADER) of=$(ATA_DISK_IMG) bs=512 count=1 seek=0
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(ATA_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(ATA_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(ATA_EXT2_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(ATA_DISK_IMG) bs=512 seek=2
	dd if=/dev/zero of=$(ATA_DISK_EXT2_IMG) bs=512 count=32256
	mkfs.ext2 -L MARCOSLIRA05 -r 0 $(ATA_DISK_EXT2_IMG)
	genext2fs -x $(ATA_DISK_EXT2_IMG) -b 4032 -B 4096 -d rootfs -v $(ATA_DISK_EXT2_IMG)
	dd conv=notrunc if=$(ATA_DISK_EXT2_IMG) of=$(ATA_DISK_IMG) bs=512 count=32256 seek=513

iso9660: rootfs
	grub-mkrescue -o $(CDROM_ISO) rootfs

vboxvdi: ataext2
	rm -f $(VBOX_VDI)
	$(VBOXMANAGE) convertfromraw --format VDI $(ATA_DISK_IMG) $(VBOX_VDI) --uuid 15a33566-0d13-4091-81ca-4ba330333b2b

vmwarevmdk: ataext2
	rm -f $(VMWARE_VMDK)
	$(VBOXMANAGE) convertfromraw --format VMDK $(ATA_DISK_IMG) $(VMWARE_VMDK)

qemu: ataext2
	$(QEMU) \
	-drive file=$(ATA_DISK_IMG),format=raw,if=ide,index=0,media=disk \
	-m 32M \
	-serial file:serial.txt \
	-netdev user,id=eth -device e1000,netdev=eth \
	-object filter-dump,id=f1,netdev=eth,file=qemu-pktlog.pcap \
	-boot c

qemuiso: iso9660
	$(QEMU) \
	-cdrom $(CDROM_ISO) \
	-m 32M \
	-serial file:serial.txt \
	-netdev user,id=eth -device e1000,netdev=eth \
	-object filter-dump,id=f1,netdev=eth,file=qemu-pktlog.pcap \
	-boot d

startfloppy: floppy
	$(BOCHS) -f ./bochsrc_floppy.bxrc -rc bochsdbg.rc -q

startata: ata
	$(BOCHS) -f ./bochsrc_ata.bxrc -rc bochsdbg.rc -q

startataext2: ataext2
	$(BOCHS) -f ./bochsrc_ata.bxrc -rc bochsdbg.rc -q

startiso: iso9660
	$(BOCHS) -f ./bochsrc_iso.bxrc -rc bochsdbg.rc -q

clean:
	make -C bootloader clean || true
	make -C kernel clean || true
	make -C system clean || true
