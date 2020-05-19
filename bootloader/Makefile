BUILD_DIR=../build/bootloader

BOOTLOADER_SRCS := $(wildcard *.asm)
BOOTLOADER_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BOOTLOADER_SRCS))

all: dir $(BUILD_DIR)/bootloader

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.asm
	nasm -f elf -F dwarf -g $< -o $@

$(BUILD_DIR)/bootloader: $(BOOTLOADER_OBJS)
	ld -m elf_i386 -nmagic -T bootloader.lds $< -o $@.elf
	objcopy -O binary -j .text $@.elf $@

clean:
	rm $(BOOTLOADER_OBJS)
