BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/bootloader/bootloader
BIOSPARAMS=$(BUILD_DIR)/bootloader/biosparams
KERNEL=$(BUILD_DIR)/kernel/kernel.elf
SYSTEM_LIB=$(BUILD_DIR)/system/lib/*.a $(BUILD_DIR)/system/lib/*.so
SYSTEM_BIN=$(BUILD_DIR)/system/bin/*/*.elf
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
GRUB2_MKIMAGE="grub-mkimage"
GRUB2_MKRESCUE="grub-mkrescue"
BOCHS="/mnt/c/Program Files/Bochs-2.8/bochsdbg.exe"
QEMU="/mnt/c/Program Files/QEMU/qemu-system-x86_64.exe"
VBOXMANAGE="/mnt/c/Program Files/Oracle/VirtualBox/VBoxManage.exe"

all: dir rootfs

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: bootloader kernel system

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
	$(GRUB2_MKIMAGE) -o $(GRUB2_CORE_IMG) -O i386-pc -p /boot/grub -c grub_ata.cfg configfile fat ext2 multiboot2 part_msdos ls nativedisk biosdisk normal help

grub2_floppy:
	$(GRUB2_MKIMAGE) -o $(GRUB2_CORE_IMG) -O i386-pc -p /boot/grub -c grub_floppy.cfg configfile fat multiboot2 part_msdos ls nativedisk biosdisk normal help

rootfs: kernel system
	mkdir -p rootfs/
	mkdir -p rootfs/boot
	mkdir -p rootfs/bin
	mkdir -p rootfs/dev
	mkdir -p rootfs/etc
	mkdir -p rootfs/lib
	mkdir -p rootfs/proc
	mkdir -p rootfs/run
	mkdir -p rootfs/sys
	mkdir -p rootfs/tmp
	mkdir -p rootfs/usr
	mkdir -p rootfs/var
	cp $(KERNEL) rootfs
	cp $(SYSTEM_BIN) rootfs/bin
	cp $(SYSTEM_LIB) rootfs/lib
	cp -f grub.cfg.example rootfs/boot/grub/grub.cfg
	cp -f fstab.example rootfs/etc/fstab

floppy: bootloader_floppy rootfs grub2_floppy
	dd if=/dev/zero of=$(FLOPPY_DISK_IMG) bs=512 count=2880
	dd conv=notrunc if=$(BOOTLOADER) of=$(FLOPPY_DISK_IMG) bs=512 count=1 seek=0
	MTOOLSRC=./mtoolsrc mformat a:
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(FLOPPY_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(FLOPPY_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(FLOPPY_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(FLOPPY_DISK_IMG) bs=512 seek=2
	sed -i 's/<rootdevice>/fd0p1/g' rootfs/boot/grub/grub.cfg
	sed -i 's/<rootdevice>/fd0p1/g' rootfs/etc/fstab
	MTOOLSRC=./mtoolsrc mlabel -n a:MARCOSLIRA
	MTOOLSRC=./mtoolsrc mcopy -m -s rootfs/* a:
	MTOOLSRC=./mtoolsrc mattrib +s a:/kernel.elf
	MTOOLSRC=./mtoolsrc mattrib +s a:/bin/init.elf
	MTOOLSRC=./mtoolsrc mattrib +s a:/lib/libc.a
	MTOOLSRC=./mtoolsrc mattrib +s a:/lib/libc.so

ata: bootloader_ata rootfs grub2_ata
	dd if=/dev/zero of=$(ATA_DISK_IMG) bs=512 count=32768
	dd conv=notrunc if=$(BOOTLOADER) of=$(ATA_DISK_IMG) bs=512 count=1 seek=0
	MTOOLSRC=./mtoolsrc mformat c:
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(ATA_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(ATA_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(ATA_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(ATA_DISK_IMG) bs=512 seek=2
	sed -i 's/<rootdevice>/hd0p1/g' rootfs/boot/grub/grub.cfg
	sed -i 's/<rootdevice>/hd0p1/g' rootfs/etc/fstab
	MTOOLSRC=./mtoolsrc mlabel -n c:MARCOSLIRA
	MTOOLSRC=./mtoolsrc mcopy -m -s rootfs/* c:
	MTOOLSRC=./mtoolsrc mattrib +s c:/kernel.elf
	MTOOLSRC=./mtoolsrc mattrib +s c:/bin/init.elf
	MTOOLSRC=./mtoolsrc mattrib +s c:/lib/libc.a
	MTOOLSRC=./mtoolsrc mattrib +s c:/lib/libc.so

ataext2: bootloader_ata_ext2 rootfs grub2_ata
	dd if=/dev/zero of=$(ATA_DISK_IMG) bs=512 count=32768
	dd conv=notrunc if=$(BOOTLOADER) of=$(ATA_DISK_IMG) bs=512 count=1 seek=0
	dd conv=notrunc if=$(GRUB2_BOOT_IMG) of=$(ATA_DISK_IMG) bs=512 count=1 seek=1
	dd conv=notrunc if=$(BIOSPARAMS) of=$(ATA_DISK_IMG) bs=1 count=94 seek=515
	dd conv=notrunc if=$(ATA_EXT2_GRUB2_CORE_SECTOR) of=$(GRUB2_CORE_IMG) bs=1 seek=500 count=1
	dd conv=notrunc if=$(GRUB2_CORE_IMG) of=$(ATA_DISK_IMG) bs=512 seek=2
	dd if=/dev/zero of=$(ATA_DISK_EXT2_IMG) bs=512 count=32256
	sed -i 's/<rootdevice>/hd0p1/g' rootfs/boot/grub/grub.cfg
	sed -i 's/<rootdevice>/hd0p1/g' rootfs/etc/fstab
	mkfs.ext2 -L MARCOSLIRA05 -O none -b 2048 -I 128 $(ATA_DISK_EXT2_IMG)
	genext2fs -x $(ATA_DISK_EXT2_IMG) -B 2048 -d rootfs -v $(ATA_DISK_EXT2_IMG)
	dd conv=notrunc if=$(ATA_DISK_EXT2_IMG) of=$(ATA_DISK_IMG) bs=512 count=32256 seek=513

iso9660: rootfs
	sed -i 's/<rootdevice>/cd0p0/g' rootfs/boot/grub/grub.cfg
	sed -i 's/<rootdevice>/cd0p0/g' rootfs/etc/fstab
	$(GRUB2_MKRESCUE) -o $(CDROM_ISO) rootfs

vboxvdi: ataext2
	rm -f $(VBOX_VDI)
	$(VBOXMANAGE) convertfromraw --format VDI $(ATA_DISK_IMG) $(VBOX_VDI) --uuid 15a33566-0d13-4091-81ca-4ba330333b2b

vmwarevmdk: ataext2
	rm -f $(VMWARE_VMDK)
	$(VBOXMANAGE) convertfromraw --format VMDK $(ATA_DISK_IMG) $(VMWARE_VMDK)

qemufloppy: floppy
	$(QEMU) \
	-drive file=$(FLOPPY_DISK_IMG),format=raw,if=floppy,index=0,media=disk \
	-m 32M \
	-serial file:serial.txt \
	-netdev user,id=eth -device e1000,netdev=eth \
	-object filter-dump,id=f1,netdev=eth,file=qemu-pktlog.pcap \
	-boot a

qemuata: ata
	$(QEMU) \
	-drive file=$(ATA_DISK_IMG),format=raw,if=ide,index=0,media=disk \
	-m 32M \
	-serial file:serial.txt \
	-netdev user,id=eth -device e1000,netdev=eth \
	-object filter-dump,id=f1,netdev=eth,file=qemu-pktlog.pcap \
	-boot c

qemuataext2: ataext2
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
