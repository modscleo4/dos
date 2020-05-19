BUILD_DIR=../build/kernel
KERNEL=$(BUILD_DIR)/kernel

CFLAGS=-ffreestanding -m32 -I libc/include -nostdlib

KERNEL_SRCS := $(wildcard *.c libc/*.c drivers/*.c)
KERNEL_HDRS := $(wildcard *.h libc/include/*.h drivers/*.h)
KERNEL_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(KERNEL_SRCS))

all: dir $(KERNEL)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	mkdir -p $(BUILD_DIR)/libc

$(BUILD_DIR)/%.o: %.c $(KERNEL_HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_OBJS)
	ld -m elf_i386 -T kernel.lds $^ -o $@

clean:
	rm $(KERNEL_OBJS)
