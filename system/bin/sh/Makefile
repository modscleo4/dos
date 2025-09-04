BUILD_DIR=../../../build/system/bin/sh
BIN=$(BUILD_DIR)/sh.elf

CC=gcc
CFLAGS=--std=gnu11 -ffreestanding -m32 -I ../../libc/include -nostdlib -nolibc -nodefaultlibs -Wl,--stack,0x4000 -Wno-packed-bitfield-compat -fno-common -fms-extensions
LDFLAGS=-m elf_i386 -Bstatic -L $(shell pwd)/../../../build/system/lib -lc

ASM_SRCS = $(wildcard *.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h ../../libc/include/*.h ../../libc/include/sys/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(BIN)

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(ASM_OBJS) $(OBJS)
	ld $^ $(LDFLAGS) -o $@

clean:
	rm -f $(OBJS)
