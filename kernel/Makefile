BUILD_DIR=../build/kernel
BIN=$(BUILD_DIR)/kernel.elf

CC=gcc
CFLAGS=-ffreestanding -O0 -g -m32 -I libc/include -fno-pic -fno-omit-frame-pointer -nostdlib -Wl,--stack,0x8000 -Wno-packed-bitfield-compat -march=pentium3 -fno-common

ASM_SRCS = $(wildcard *.asm cpu/*.asm modules/*.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c libc/*.c drivers/*.c drivers/eth/*.c drivers/fs/*.c drivers/io/*.c drivers/video/*.c cpu/*.c modules/*.c modules/kblayout/*.c modules/net/*.c)
HDRS := $(wildcard *.h libc/include/*.h drivers/*.h drivers/eth/*.c drivers/fs/*.h drivers/io/*.h drivers/video/*.h cpu/*.h modules/*.h modules/kblayout/*.h modules/net/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(BIN)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	mkdir -p $(BUILD_DIR)/drivers/eth
	mkdir -p $(BUILD_DIR)/drivers/fs
	mkdir -p $(BUILD_DIR)/drivers/io
	mkdir -p $(BUILD_DIR)/drivers/video
	mkdir -p $(BUILD_DIR)/libc
	mkdir -p $(BUILD_DIR)/cpu
	mkdir -p $(BUILD_DIR)/modules
	mkdir -p $(BUILD_DIR)/modules/net
	mkdir -p $(BUILD_DIR)/modules/kblayout

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(ASM_OBJS) $(OBJS)
	ld -g -m elf_i386 -T kernel.lds $^ -o $@

clean:
	rm -f $(OBJS) $(ASM_OBJS)
