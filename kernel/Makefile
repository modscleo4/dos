BUILD_DIR=../build/kernel
BIN=$(BUILD_DIR)/kernel

CFLAGS=-ffreestanding -g -m32 -I libc/include -fno-omit-frame-pointer -nostdlib -Wl,--stack,0x4000 -Wno-packed-bitfield-compat

ASM_SRCS = $(wildcard *.asm cpu/*.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c libc/*.c drivers/*.c drivers/fs/*.c cpu/*.c modules/*.c modules/net/*.c)
HDRS := $(wildcard *.h libc/include/*.h drivers/*.h drivers/fs/*.h cpu/*.h modules/*.h modules/kblayout/*.h modules/net/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(BIN)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	mkdir -p $(BUILD_DIR)/drivers/fs
	mkdir -p $(BUILD_DIR)/libc
	mkdir -p $(BUILD_DIR)/cpu
	mkdir -p $(BUILD_DIR)/modules
	mkdir -p $(BUILD_DIR)/modules/net

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(BIN): $(ASM_OBJS) $(OBJS)
	ld -g -m elf_i386 -T kernel.lds $^ -o $@

clean:
	rm -f $(OBJS) $(ASM_OBJS)
