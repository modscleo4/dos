BUILD_DIR=../build/kernel
KERNEL=$(BUILD_DIR)/kernel

CFLAGS=-ffreestanding -m32 -I libc/include -nostdlib

KERNEL_ASM_SRCS = $(wildcard *.asm cpu/*.asm)
KERNEL_ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(KERNEL_ASM_SRCS))
KERNEL_SRCS := $(wildcard *.c libc/*.c drivers/*.c cpu/*.c modules/*.c)
KERNEL_HDRS := $(wildcard *.h libc/include/*.h drivers/*.h cpu/*.h modules/*.h modules/kblayout/*.h)
KERNEL_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(KERNEL_SRCS))

all: dir $(KERNEL)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	mkdir -p $(BUILD_DIR)/libc
	mkdir -p $(BUILD_DIR)/cpu
	mkdir -p $(BUILD_DIR)/modules

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(KERNEL_HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_ASM_OBJS) $(KERNEL_OBJS)
	ld -m elf_i386 -T kernel.lds $^ -o $@

clean:
	rm $(KERNEL_OBJS) $(KERNEL_ASM_OBJS)
