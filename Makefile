BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/bootloader/bootloader
KERNEL=$(BUILD_DIR)/kernel/kernel
SYSTEM_INIT=$(BUILD_DIR)/system/init/init.elf
FLOPPY_DISK_IMG=floppy_disk.img
ATA_DISK_IMG=ata_disk.img
VBOX_DISK_IMG=ata_disk.vdi

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

kernel:
	make -C kernel

system:
	make -C system

floppy: bootloader_floppy kernel system
	dd if=/dev/zero of=$(FLOPPY_DISK_IMG) bs=512 count=2880
	#mkfs.vfat $(FLOPPY_DISK_IMG)
	dd conv=notrunc if=$(BOOTLOADER) of=$(FLOPPY_DISK_IMG) bs=512 count=1 seek=0
	#dd conv=notrunc if=$(KERNEL) of=$(FLOPPY_DISK_IMG) bs=512 count=128 seek=33
	mcopy $(KERNEL) a:
	mcopy $(SYSTEM_INIT) a:

ata: bootloader_ata kernel system
	dd if=/dev/zero of=$(ATA_DISK_IMG) bs=512 count=32768
	#mkfs.vfat $(ATA_DISK_IMG) -F 16
	dd conv=notrunc if=$(BOOTLOADER) of=$(ATA_DISK_IMG) bs=512 count=1 seek=0
	#dd conv=notrunc if=$(KERNEL) of=$(ATA_DISK_IMG) bs=512 count=128 seek=33
	export MTOOLSRC="./mtoolsrc"
	mcopy $(KERNEL) c:
	mcopy $(SYSTEM_INIT) c:

vboxvdi: ata
	rm -f $(VBOX_DISK_IMG)
	"/mnt/c/Program Files/Oracle/VirtualBox/VBoxManage.exe" convertfromraw --format VDI $(ATA_DISK_IMG) $(VBOX_DISK_IMG) --uuid 15a33566-0d13-4091-81ca-4ba330333b2b

startfloppy: floppy
	"/mnt/c/Program Files/Bochs-2.7/bochsdbg.exe" -f ./bochsrc_floppy.bxrc -rc bochsdbg.rc -q
	#"/mnt/c/Program Files/Oracle/VirtualBox/VirtualBoxVM.exe" --comment "SO" --startvm "{b1e3976b-9a74-4224-b868-fc050192db27}"

startata: ata
	"/mnt/c/Program Files/Bochs-2.7/bochsdbg.exe" -f ./bochsrc_ata.bxrc -rc bochsdbg.rc -q
	#"/mnt/c/Program Files/Oracle/VirtualBox/VirtualBoxVM.exe" --comment "SO" --startvm "{b1e3976b-9a74-4224-b868-fc050192db27}"

clean:
	make -C bootloader clean || true
	make -C kernel clean || true
	make -C system clean || true
