BUILD_DIR=../build/bootloader

BOOTLOADER_SRCS := $(wildcard bootloader*.asm)
BOOTLOADER_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BOOTLOADER_SRCS))

BIOSPARAMS_SRCS := $(wildcard biosparams*.asm)
BIOSPARAMS_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BIOSPARAMS_SRCS))

all: dir $(BUILD_DIR)/bootloader $(BUILD_DIR)/biosparams

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.asm
	nasm -f elf -F dwarf -g $< -o $@ $(ARG1)

$(BUILD_DIR)/bootloader: $(BOOTLOADER_OBJS)
	ld -m elf_i386 -nmagic -T bootloader.lds $< -o $@.elf
	objcopy -O binary -j .text $@.elf $@

$(BUILD_DIR)/biosparams: $(BIOSPARAMS_OBJS)
	ld -m elf_i386 -nmagic -T biosparams.lds $< -o $@.elf
	objcopy -O binary -j .text $@.elf $@

clean:
	rm -f $(BUILD_DIR)/*
