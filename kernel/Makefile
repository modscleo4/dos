BUILD_DIR=../build/kernel
BIN=$(BUILD_DIR)/kernel

CFLAGS=-ffreestanding -m32 -I libc/include -nostdlib

ASM_SRCS = $(wildcard *.asm cpu/*.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c libc/*.c drivers/*.c cpu/*.c modules/*.c)
HDRS := $(wildcard *.h libc/include/*.h drivers/*.h cpu/*.h modules/*.h modules/kblayout/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(BIN)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	mkdir -p $(BUILD_DIR)/libc
	mkdir -p $(BUILD_DIR)/cpu
	mkdir -p $(BUILD_DIR)/modules

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(BIN): $(ASM_OBJS) $(OBJS)
	ld -m elf_i386 -T kernel.lds $^ -o $@

clean:
	rm $(OBJS) $(ASM_OBJS)
