BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/bootloader/bootloader
FAT=$(BUILD_DIR)/fat/fat
ROOTDIR=$(BUILD_DIR)/fat/rootdir
KERNEL=$(BUILD_DIR)/kernel/kernel
DISK_IMG=disk.img

all: dir disk

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: disk bootloader kernel fat rootdir

bootloader:
	make -C bootloader

kernel:
	make -C kernel

disk: bootloader kernel
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=2880
	mkfs.vfat $(DISK_IMG)
	dd conv=notrunc if=$(BOOTLOADER) of=$(DISK_IMG) bs=512 count=1 seek=0
	mcopy -i $(DISK_IMG) $(KERNEL) ::/

startvm: disk
	"/mnt/c/Program Files/Oracle/VirtualBox/VirtualBoxVM.exe" --comment "SO" --startvm "{b1e3976b-9a74-4224-b868-fc050192db27}"

clean:
	make -C bootloader clean || true
	make -C kernel clean || true
