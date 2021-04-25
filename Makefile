BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/bootloader/bootloader
KERNEL=$(BUILD_DIR)/kernel/kernel
SYSTEM_SHELL=$(BUILD_DIR)/system/shell/shell
DISK_IMG=disk.img

all: dir disk

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: disk bootloader kernel system

bootloader:
	make -C bootloader

kernel:
	make -C kernel

system:
	make -C system

disk: bootloader kernel system
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=2880
	mkfs.vfat $(DISK_IMG)
	dd conv=notrunc if=$(BOOTLOADER) of=$(DISK_IMG) bs=512 count=1 seek=0
	mcopy -i $(DISK_IMG) $(KERNEL) ::/
	mcopy -i $(DISK_IMG) $(SYSTEM_SHELL) ::/

startvm: disk
	"/mnt/c/Program Files/Bochs/bochsdbg.exe" -f "C:\\Users\\Modscleo4\\Documents\\Bochs\\bochsrc.bxrc" -q
	#"/mnt/c/Program Files/Oracle/VirtualBox/VirtualBoxVM.exe" --comment "SO" --startvm "{b1e3976b-9a74-4224-b868-fc050192db27}"

clean:
	make -C bootloader clean || true
	make -C kernel clean || true
	make -C system clean || true
